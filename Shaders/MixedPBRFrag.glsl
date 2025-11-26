#version 330 core

uniform sampler2D diffuseTex;
uniform sampler2D bumpTex;
uniform sampler2D noiseTex;
uniform sampler2D roughnessTex;

uniform int hasRoughness;
uniform int hasBump;

in Vertex {
	vec4 colour;
	vec2 texCoord;
	vec3 normal;
	vec3 tangent;
	vec3 binormal;
	vec3 worldPos;
} IN;

// 0: Colour, 1: Normal, 2: Emissive (ignored here)
out vec4 fragColour[3];

void main(void) {
	vec4 colour = texture(diffuseTex, IN.texCoord);

	// R = Ambient Occlusion G = Roughness B = Metallic
	vec3 pbrData = vec3(0, 1, 0);
	if (hasRoughness > 0)
		pbrData = texture(roughnessTex, IN.texCoord).rgb;
	
	float ao = pbrData.r;
	float roughness = pbrData.g;
	float metallic = pbrData.b;

	colour.rgb *= ao;

	// Metallic in diffuse alpha 
	// Range 0.1 - 1.0 so its not discarded
	fragColour[0] = vec4(colour.rgb, 0.1 + (metallic * 0.9)); 

	mat3 TBN = mat3(normalize(IN.tangent), normalize(IN.binormal), normalize(IN.normal));

	vec3 normal = normalize(IN.normal);
	if (hasBump > 0) {
		normal = texture2D(bumpTex, IN.texCoord).rgb * 2.0 - 1.0; // range: -1 to 1
		normal = normalize(TBN * normalize(normal));
	}

	fragColour[1] = vec4(normal.xyz * 0.5 + 0.5, roughness); // range: 0 to 1
	fragColour[2] = vec4(0, 0, 0, 1); 
}