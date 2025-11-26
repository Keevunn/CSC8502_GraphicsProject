#version 400

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projMatrix;

layout(location = 0) in vec3 position;
layout(location = 2) in vec2 texCoord;
in vec3 normal;
in vec4 tangent;
layout(location = 5) in vec4 jointWeights;
layout(location = 6) in ivec4 jointIndices;

uniform mat4 joints[128];

out Vertex {
	vec4 colour;
	vec2 texCoord;
	vec3 normal;
	vec3 tangent;
	vec3 binormal;
	vec3 worldPos;
} OUT;

void main(void) {
	vec4 localPos = vec4(position, 1.0f);
	vec4 skelPos = vec4(0, 0, 0, 0);

	for (int i = 0; i < 4; ++i) {
		int jointIndex = jointIndices[i];
		float jointWeight = jointWeights[i];

		if (jointIndex == -1)
			continue;
		if (jointIndex >= 128) {
			skelPos = localPos;
			break;
		}

		skelPos += joints[jointIndex] * localPos * jointWeight;
	}
	
	mat4 mvp = projMatrix * viewMatrix * modelMatrix;
	gl_Position = mvp * skelPos;

	mat3 normalMatrix = transpose(inverse(mat3(modelMatrix)));
	
	vec3 wNormal = normalize(normalMatrix * normalize(normal));
	OUT.normal = wNormal;
	vec3 wTangent = normalize(normalMatrix * normalize(tangent.xyz));
	OUT.tangent = wTangent;
	OUT.binormal = cross(wTangent, wNormal) * tangent.w;

	OUT.texCoord = texCoord;
}