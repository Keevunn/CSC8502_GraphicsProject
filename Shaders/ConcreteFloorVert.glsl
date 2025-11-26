#version 410 core

in vec3 position;
in vec3 normal;
in vec2 texCoord;

// Output to Tessellation control shader
out vec3 worldPos_CSin;
out vec2 texCoord_CSin;
out vec3 normal_CSin;

void main() {
	worldPos_CSin = position;
	texCoord_CSin = texCoord;
	normal_CSin = normal;
}