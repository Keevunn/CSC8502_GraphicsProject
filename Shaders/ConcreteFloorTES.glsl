#version 410 core

layout(triangles, equal_spacing, ccw) in;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projMatrix;

uniform sampler2D displacementTex;
uniform float displacementStrength; 
uniform int texWidth; 

in vec3 worldPos_ESin[];
in vec2 texCoord_ESin[];
in vec3 normal_ESin[];

out vec3 worldPos_FSin;
out vec2 texCoord_FSin;

out vec3 normal_FSin;
out vec3 tangent_FSin;
out vec3 bitangent_FSin;

// Interpolate vectors based on barycentric coords (gl_TessCoord)

vec3 Interpolate3D(vec3 v0, vec3 v1, vec3 v2) {
	return gl_TessCoord.x * v0 + gl_TessCoord.y * v1 + gl_TessCoord.z * v2;
}

vec2 Interpolate2D(vec2 v0, vec2 v1, vec2 v2) {
	return gl_TessCoord.x * v0 + gl_TessCoord.y * v1 + gl_TessCoord.z * v2;
}

void CalculateNewNormal(vec2 texCoord, float height) {
	float offset = 1.0 / texWidth;

	// Sample neighbours
	float heightRight = texture(displacementTex, texCoord + vec2(offset, 0)).r;
	float heightUp = texture(displacementTex, texCoord + vec2(0, offset)).r;

	// Calculate tangent and bitanget
	tangent_FSin = normalize(vec3(1.0, (heightRight - height) * displacementStrength, 0)); // 1 represents distance between samples in world space (might need to change)
	bitangent_FSin = normalize(vec3(0.0, (heightUp - height) * displacementStrength, -1));

	normal_FSin = normalize(cross(tangent_FSin, bitangent_FSin));
}

void main() {
	// Interpolate attibutes of new vertex
	vec3 pos = Interpolate3D(worldPos_ESin[0], worldPos_ESin[1], worldPos_ESin[2]);
	vec3 normal = normalize(Interpolate3D(normal_ESin[0], normal_ESin[1], normal_ESin[2]));
	vec2 texCoord = Interpolate2D(texCoord_ESin[0], texCoord_ESin[1], texCoord_ESin[2]);

	float height = texture(displacementTex, texCoord).r;

	// Displace vertex along normal
	pos += normal * height * displacementStrength;

	mat4 mvp = projMatrix * viewMatrix * modelMatrix;
	gl_Position = mvp * vec4(pos, 1);

	worldPos_FSin = pos;
	texCoord_FSin = texCoord;
	CalculateNewNormal(texCoord, height);
}