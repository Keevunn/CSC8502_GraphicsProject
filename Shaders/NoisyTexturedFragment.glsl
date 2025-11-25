#version 330 core

uniform sampler2D diffuseTex;
uniform sampler2D bumpTex;
uniform sampler2D noiseTex;
uniform sampler2D roughnessTex;

uniform bool useNoise;
uniform bool hasRoughness;
uniform bool hasBump;

in Vertex {
	vec4 colour;
	vec2 texCoord;
	vec3 normal;
	vec3 tangent;
	vec3 binormal;
	vec3 worldPos;
} IN;

out vec4 fragColour[2];

void main(void) {
	vec4 colour = texture(diffuseTex, IN.texCoord);
	if (colour.a < 0.1)
		discard;

	if (useNoise) {
		// different scale so it doesn't line up with diffuseTex
		float noiseValue = texture(noiseTex, IN.texCoord * 0.125).r; 
		//mix(original, darker, strength)
		colour.rgb = mix(colour.rgb, colour.rbg * 0.4, noiseValue); 
	}
	fragColour[0] = vec4(colour.rgb, 1);

	mat3 TBN = mat3(normalize(IN.tangent), normalize(IN.binormal), normalize(IN.normal));

	vec3 normal = normalize(IN.normal);
	if (hasBump) {
		normal = texture2D(bumpTex, IN.texCoord).rgb * 2.0 - 1.0; // range: -1 to 1
		normal = normalize(TBN * normalize(normal));
	}

	// Roughness stored in alpha channel
	float roughness;
	if (hasRoughness)
		roughness = texture(roughnessTex, IN.texCoord).r;
	else
		roughness = 1;
	fragColour[1] = vec4(normal.xyz * 0.5 + 0.5, roughness); // range: 0 to 1
}