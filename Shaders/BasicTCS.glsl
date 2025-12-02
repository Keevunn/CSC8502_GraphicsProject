#version 410 core

layout (vertices = 3) out; // outputs 3 control points per patch

// Input data from vertex shader, automatically grouped into patches
in vec3 worldPos_CSin[];
in vec2 texCoord_CSin[];
in vec3 normal_CSin[];

// Output to evaluation shader
out vec3 worldPos_ESin[];
out vec2 texCoord_ESin[];
out vec3 normal_ESin[];

uniform float tessLevel;

void main() {
	worldPos_ESin[gl_InvocationID] = worldPos_CSin[gl_InvocationID];
	texCoord_ESin[gl_InvocationID] = texCoord_CSin[gl_InvocationID];
	normal_ESin[gl_InvocationID] = normal_CSin[gl_InvocationID];

	// Calculate tessellation level for invocation 0
	if (gl_InvocationID == 0) {
		gl_TessLevelOuter[0] = tessLevel;
		gl_TessLevelOuter[1] = tessLevel;
		gl_TessLevelOuter[2] = tessLevel;
		gl_TessLevelInner[0] = tessLevel;
	}
}