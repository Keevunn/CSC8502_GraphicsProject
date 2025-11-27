#version 330 core

uniform sampler2D depthTex;
uniform sampler2D normTex; // roughness in alpha channel

uniform vec2 pixelSize;
uniform vec3 cameraPos;

uniform vec3 lightPos;
uniform vec3 lightDir; // lightSource -> fragment
uniform vec4 lightDiffColour;
uniform vec4 lightSpecColour;
uniform float lightInnerCutoff; // cos(angle)
uniform float lightOuterCutoff; // cos(angle)
uniform mat4 inverseProjView;

// Attenuation values
uniform vec3 attenValues; // constant, linear, quadratic


out vec4 diffuseOutput;
out vec4 specularOutput;

void main() {
	vec2 texCoord = vec2(gl_FragCoord.xy * pixelSize);
	float depth = texture(depthTex, texCoord.xy).r;

	vec3 ndcPos = vec3(texCoord, depth) * 2 - 1;
	vec4 invClipPos = inverseProjView * vec4(ndcPos, 1);
	vec3 worldPos = invClipPos.xyz / invClipPos.w;

	vec3 incident = normalize(lightPos - worldPos);
	vec3 viewDir = normalize(cameraPos - worldPos);
	vec3 halfDir = normalize(incident + viewDir);
	

	// cos(theta), theta - angle between incident and light direction
	float theta = dot(incident, normalize(-lightDir)); // No light, out of volume
	float epsilon = lightInnerCutoff - lightOuterCutoff;

	float intensity = clamp((theta - lightOuterCutoff) / epsilon, 0, 1); // 0 when outside light
	if (intensity == 0) discard;

	// Attenuation calculation
	float dist = length(lightPos - worldPos);
	float constant = attenValues.x;
	float linear = attenValues.y;
	float quadratic = attenValues.z;
	float atten = 1.0 / (constant + linear * dist + quadratic * (dist * dist));

	vec4 normalData = texture(normTex, texCoord.xy);
	vec3 normal = normalize(normalData.xyz * 2 - 1);
	float roughness = normalData.a;

	// Roughness (0 = Glossy, 1 = Matte) -> Specular power (High = Glossy, Low = Matte)
	float shininess = mix(60, 1, roughness);

	float lambert = clamp(dot(incident, normal), 0, 1);
	float rFactor = clamp(dot(halfDir, normal), 0, 1);
	float specFactor = pow(rFactor, shininess);
	float specularIntensity = 1.0 - roughness;

	//vec3 attenuated = lightColour.rgb * intensity;
	diffuseOutput = vec4(lightDiffColour.rgb * intensity * atten * lambert, 1);
	specularOutput = vec4(lightSpecColour.rgb * intensity * atten * specFactor * specularIntensity, 1);
}