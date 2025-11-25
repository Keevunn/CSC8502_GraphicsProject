#version 330 core

uniform sampler2D diffuseTex;
uniform sampler2D bumpTex;
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

out vec4 fragColour[2];

void main(void) {
	vec4 colour = texture(diffuseTex, IN.texCoord);
	if (colour.a < 0.1)
		discard;
	fragColour[0] = colour;

	mat3 TBN = mat3(normalize(IN.tangent), normalize(IN.binormal), normalize(IN.normal));

	vec3 normal = normalize(IN.normal);
	if (hasBump == 1) {
		normal = texture2D(bumpTex, IN.texCoord).rgb * 2.0 - 1.0; // range: -1 to 1
		normal = normalize(TBN * normalize(normal));
	}
	

	// Roughness stored in alpha channel
	float roughness = 1;
	if (hasRoughness == 1)
		roughness = texture(roughnessTex, IN.texCoord).r;
	fragColour[1] = vec4(normal.xyz * 0.5 + 0.5, roughness); // range: 0 to 1
}