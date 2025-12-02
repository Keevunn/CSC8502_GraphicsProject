#version 410 core

uniform sampler2D blendMap;
uniform float texScale; 

// Base Texture
uniform sampler2D baseDiffuseTex;
uniform sampler2D baseBumpTex;
uniform sampler2D baseAOTex;
uniform sampler2D baseRoughnessTex;

// Sand textures
uniform sampler2D sandDiffuseTex;
uniform sampler2D sandBumpTex;
uniform sampler2D sandAOTex;
uniform sampler2D sandRoughnessTex;

// Grass textures
uniform sampler2D grassDiffuseTex;
uniform sampler2D grassBumpTex;
uniform sampler2D grassAOTex;
uniform sampler2D grassRoughnessTex;

// Rock textures
uniform sampler2D rockDiffuseTex;
uniform sampler2D rockBumpTex;
uniform sampler2D rockAOTex;
uniform sampler2D rockRoughnessTex;
uniform sampler2D rockMetallicTex;

in vec3 worldPos_FSin;
in vec2 texCoord_FSin;
in vec3 normal_FSin;
in vec3 tangent_FSin;
in vec3 bitangent_FSin;

// 0: Colour, 1: Normal, 2: Emissive (ignored here)
out vec4 fragColour[3];


void main() {
	vec3 blend = texture(blendMap, texCoord_FSin).rgb;
	float baseWeight = clamp(1.0 - (blend.r + blend.g + blend.b), 0.0, 1.0);
	vec2 tiledUV = texCoord_FSin * texScale;

	// Sample colours
	vec3 baseColour = texture(baseDiffuseTex, tiledUV).rgb;
	vec3 baseAO = texture(baseAOTex, tiledUV).rgb;

	vec3 sandColour = texture(sandDiffuseTex, tiledUV).rgb;
	vec3 sandAO = texture(sandAOTex, tiledUV).rgb;

	vec3 grassColour = texture(grassDiffuseTex, tiledUV).rgb;
	vec3 grassAO = texture(grassAOTex, tiledUV).rgb;

	vec3 rockColour = texture(rockDiffuseTex, tiledUV).rgb;
	vec3 rockAO = texture(rockAOTex, tiledUV).rgb;

	vec3 finalColour = (baseColour * baseAO * baseWeight) + (sandColour * sandAO * blend.r) + (grassColour * grassAO * blend.g) + (rockColour * rockAO * blend.b);

	// Sample normals
	mat3 TBN = mat3(normalize(tangent_FSin), normalize(bitangent_FSin), normalize(normal_FSin));

	vec3 baseNormal = texture(baseBumpTex, tiledUV).rgb * 2.0 - 1.0;
	vec3 sandNormal = texture(sandBumpTex, tiledUV).rgb * 2.0 - 1.0;
	vec3 grassNormal = texture(grassBumpTex, tiledUV).rgb * 2.0 - 1.0;
	vec3 rockNormal = texture(rockBumpTex, tiledUV).rgb * 2.0 - 1.0;

	vec3 finalNormal = normalize((baseNormal * baseWeight) + (sandNormal * blend.r) + (grassNormal * blend.g) + (rockNormal * blend.b));
	finalNormal = normalize(TBN * normalize(finalNormal));

	// Sample roughness
	float baseRoughness = texture(baseRoughnessTex, tiledUV).r;
	float sandRoughness = texture(sandRoughnessTex, tiledUV).r;
	float grassRoughness = texture(grassRoughnessTex, tiledUV).r;
	float rockRoughness = texture(rockRoughnessTex, tiledUV).r;

	float finalRoughness = (baseRoughness * baseWeight) + (sandRoughness * blend.r) + (grassRoughness * blend.g) + (rockRoughness * blend.b);
	finalRoughness = clamp(finalRoughness + 0.2, 0, 1);
		
	// Sample metallic
	float rockMetallic = texture(rockMetallicTex, tiledUV).r;
	float finalMetalness = rockMetallic * blend.b * 0.15; // Reduces shineness of rocks

	fragColour[0] = vec4(finalColour, 0.1 + (finalMetalness * 0.9));
	fragColour[1] = vec4(finalNormal * 0.5 + 0.5, finalRoughness);
	fragColour[2] = vec4(0);
}