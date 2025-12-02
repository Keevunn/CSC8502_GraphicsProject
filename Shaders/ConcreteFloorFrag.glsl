#version 330 core

uniform sampler2D diffuseTex;
uniform sampler2D aoTex;
uniform sampler2D bumpTex;
uniform sampler2D roughnessTex;

uniform sampler2D noiseTex;

in vec3 worldPos_FSin;
in vec2 texCoord_FSin;

in vec3 normal_FSin;
in vec3 tangent_FSin;
in vec3 bitangent_FSin;

// 0: Colour, 1: Normal, 2: Emissive (ignored here)
out vec4 fragColour[3];

void main(void) {
	vec4 colour = texture(diffuseTex, texCoord_FSin);
	float ao = texture(aoTex, texCoord_FSin).r;
	float roughness = texture(roughnessTex, texCoord_FSin).r;

	// Bake in ambient occlusion
	colour.rgb *= ao;

	// Dirt (noise added in)
	// different scale so it doesn't line up with diffuseTex
	// Hardcoded: 1/50 so it spans over the whole ground
	float noiseValue = texture(noiseTex, texCoord_FSin * (1.0/50.0)).r; 
	//mix(original, darker, strength)
	colour.rgb = mix(colour.rgb, colour.rbg * 0.4, noiseValue); 

	fragColour[0] = vec4(colour.rgb, 0.1);  // Metallic value is 0.1 so not discarded in combine frag

	mat3 TBN = mat3(normalize(tangent_FSin), normalize(bitangent_FSin), normalize(normal_FSin));

	//vec3 normal = normalize(normal_FSin);
	vec3 normal = texture2D(bumpTex, texCoord_FSin).rgb * 2.0 - 1.0; // range: -1 to 1
	normal = normalize(TBN * normalize(normal));

	// roughness stored in normal alpha (used in light pass)
	fragColour[1] = vec4(normal.xyz * 0.5 + 0.5, roughness); // range: 0 to 1
	fragColour[2] = vec4(0, 0, 0, 1); 
}