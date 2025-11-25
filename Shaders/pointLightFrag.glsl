#version 330 core

uniform sampler2D depthTex;
uniform sampler2D normTex; // roughness in alpha channel

uniform vec2 pixelSize;
uniform vec3 cameraPos;

uniform float lightRadius;
uniform vec3 lightPos;
uniform vec4 lightColour;
uniform mat4 inverseProjView;

out vec4 diffuseOutput;
out vec4 specularOutput;

void main() {
	vec2 texCoord = vec2(gl_FragCoord.xy * pixelSize);
	float depth = texture(depthTex, texCoord.xy).r;

	vec3 ndcPos = vec3(texCoord, depth) * 2 - 1;
	vec4 invClipPos = inverseProjView * vec4(ndcPos, 1);
	vec3 worldPos = invClipPos.xyz / invClipPos.w;

	float dist = length(lightPos - worldPos);
	float atten = 1 - clamp(dist/lightRadius, 0, 1);
	if (atten == 0) {
		discard;
	}

	vec4 normalData = texture(normTex, texCoord.xy);
	vec3 normal = normalize(normalData.xyz * 2 - 1);
	float roughness = normalData.a;

	vec3 incident = normalize(lightPos - worldPos);
	vec3 viewDir = normalize(cameraPos - worldPos);
	vec3 halfDir = normalize(incident + viewDir);

	// Roughness (0 = Glossy, 1 = Matte) -> Specular power (High = Glossy, Low = Matte)
	float shininess = mix(60, 1, roughness);

	float lambert = clamp(dot(incident, normal), 0, 1);
	float rFactor = clamp(dot(halfDir, normal), 0, 1);
	float specFactor = pow(rFactor, shininess);
	float specularIntensity = 1.0 - roughness;

	vec3 attenuated = lightColour.rgb * atten;
	diffuseOutput = vec4(attenuated * lambert, 1);
	specularOutput = vec4(attenuated * specFactor * specularIntensity, 1);
}