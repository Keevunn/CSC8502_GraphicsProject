#version 330 core

uniform sampler2D depthTex;
uniform sampler2D normTex; // roughness in alpha channel

uniform vec2 pixelSize;
uniform vec3 cameraPos;

uniform vec3 lightDir;
uniform vec4 lightColour;
uniform mat4 inverseProjView;

out vec4 diffuseOutput;
out vec4 specularOutput;

void main() {
	vec2 texCoord = vec2(gl_FragCoord.xy * pixelSize);
	float depth = texture(depthTex, texCoord.xy).r;
	if (depth == 1) discard;


	vec3 ndcPos = vec3(texCoord, depth) * 2 - 1;
	vec4 invClipPos = inverseProjView * vec4(ndcPos, 1);
	vec3 worldPos = invClipPos.xyz / invClipPos.w;

	vec4 normalData = texture(normTex, texCoord.xy);
	vec3 normal = normalize(normalData.xyz * 2 - 1);
	float roughness = max(normalData.a, 0.05); // no surface is perfectly smooth

	vec3 incident = normalize(-lightDir);
	vec3 viewDir = normalize(cameraPos - worldPos);
	vec3 halfDir = normalize(incident + viewDir);

	// Roughness (0 = Glossy, 1 = Matte) -> Specular power (High = Glossy, Low = Matte)
	float shininess = mix(128, 1, roughness);

	float lambert = clamp(dot(incident, normal), 0, 1);
	float rFactor = clamp(dot(halfDir, normal), 0, 1);
	float specFactor = pow(rFactor, shininess);
	float specularIntensity = 1.0 - roughness;

	diffuseOutput = vec4(lightColour.rgb * lambert, 1);
	specularOutput = vec4(lightColour.rgb * specFactor * specularIntensity, 1);
}