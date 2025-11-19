#version 430 core

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projMatrix;

in vec3 position;
in vec2 texCoord;
in vec4 jointWeights;
in ivec4 boneIDs;

uniform mat4 joints[128];

out Vertex {
	vec2 texCoord;
} OUT;

void main() {
	vec4 skelPos = vec4(0);
	
	for (int i = 0; i < 4; ++i) {
		if (boneIDs[i] == -1)
			continue;
		if (boneIDs[i] >= 128) {
			skelPos = vec4(position, 1);
			break;
		}
		vec4 localPos = joints[boneIDs[i]] * vec4(position, 1);
		skelPos += localPos * weights[i];
	}
	
	mat4 mvp = projMatrix * viewMatrix * modelMatrix;
	gl_Position = mvp * vec4(skelPos.xyz, 1);
	OUT.texCoord = texCoord;
}