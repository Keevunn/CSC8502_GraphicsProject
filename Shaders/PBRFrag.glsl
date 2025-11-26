#version 330 core

uniform sampler2D diffuseTex;
uniform sampler2D alphaTex;
uniform sampler2D bumpTex;
uniform sampler2D roughnessTex;
uniform sampler2D metallicTex;
uniform sampler2D emissiveTex;

uniform int hasMixedPBR;

uniform int hasOpacity;
uniform int hasBump;
uniform int hasRoughness;
uniform int hasMetallic;
uniform int hasEmissive;

uniform float emissionIntensity; // Controls brightness
uniform float time;

in Vertex {
	vec4 colour;
	vec2 texCoord;
	vec3 normal;
	vec3 tangent;
	vec3 binormal;
	vec3 worldPos;
} IN;

// 0: Colour, 1: Normal, 2: Emissive
out vec4 fragColour[3];

void main(void) {
	if (hasOpacity > 0) {
		float opacity = texture(alphaTex, IN.texCoord).r;
		if (opacity < 0.1) discard;
	}

	vec4 colour = texture(diffuseTex, IN.texCoord);
	
	float roughness = 1;
	float metallic = 0;

	if (hasMixedPBR > 0) {
		// R = Ambient Occlusion G = Roughness B = Metallic
		vec3 pbrData = vec3(0, 1, 0); // ao is 0 so it's clear if hasRoughness is set incorrectly 
		if (hasRoughness > 0)
			pbrData = texture(roughnessTex, IN.texCoord).rgb;
	
		float ao = pbrData.r;
		float roughness = pbrData.g;
		float metallic = pbrData.b;

		colour.rgb *= ao; // maybe square ao (test)
	} else {
		if (hasRoughness > 0) roughness = texture(roughnessTex, IN.texCoord).r;
		if (hasMetallic > 0) metallic = texture(metallicTex, IN.texCoord).r;
	}

	// Metallic in colour alpha 
	// Range 0.1 - 1.0 so its not discarded
	fragColour[0] = vec4(colour.rgb, 0.1 + (metallic * 0.9)); 

	// Calculate normals
	mat3 TBN = mat3(normalize(IN.tangent), normalize(IN.binormal), normalize(IN.normal));

	vec3 normal = normalize(IN.normal);
	if (hasBump > 0) {
		normal = texture2D(bumpTex, IN.texCoord).rgb * 2.0 - 1.0; // range: -1 to 1
		normal = normalize(TBN * normalize(normal));
	}

	// Roughness in normal alpha
	fragColour[1] = vec4(normal.xyz * 0.5 + 0.5, roughness); // range: 0 to 1

	// Emissive output
	fragColour[2] = vec4(0, 0, 0, 1);
	if (hasEmissive > 0) {
		// Range: 0 - 1
		float pulse = (sin(time * 3.0) * 0.5) + 0.5;

		vec3 glow = texture(emissiveTex, IN.texCoord).rgb;
		fragColour[2] = vec4(glow * emissionIntensity * pulse, 1);
	}
}