#version 330 core

uniform sampler2D diffuseTex;
uniform sampler2D bumpTex;
uniform sampler2D roughnessTex;
uniform sampler2D metallicTex;
uniform sampler2D alphaTex;

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
	float opacity = texture(alphaTex, IN.texCoord).r;
	if (opacity < 0.1) discard;


	vec4 colour = texture(diffuseTex, IN.texCoord);

	float roughness = texture(roughnessTex, IN.texCoord).r;
	float metallic = texture(metallicTex, IN.texCoord).r;

	// Metallic in diffuse alpha 
	// Range 0.1 - 1.0 so its not discarded
	fragColour[0] = vec4(colour.rgb, 0.1 + (metallic * 0.9)); 

	mat3 TBN = mat3(normalize(IN.tangent), normalize(IN.binormal), normalize(IN.normal));

	vec3 normal = normalize(IN.normal);
	normal = texture2D(bumpTex, IN.texCoord).rgb * 2.0 - 1.0; // range: -1 to 1
	normal = normalize(TBN * normalize(normal));

	fragColour[1] = vec4(normal.xyz * 0.5 + 0.5, roughness); // range: 0 to 1
}