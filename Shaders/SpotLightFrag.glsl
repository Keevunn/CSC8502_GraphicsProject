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
uniform vec4 attenValues; // range, constant, linear, quadratic

out vec4 diffuseOutput;
out vec4 specularOutput;

void main() {
	vec2 texCoord = vec2(gl_FragCoord.xy * pixelSize);

	float depth = texture(depthTex, texCoord.xy).r;
	vec3 ndcPos = vec3(texCoord, depth) * 2 - 1;
	vec4 invClipPos = inverseProjView * vec4(ndcPos, 1);
	vec3 worldPos = invClipPos.xyz / invClipPos.w;

	float coneDepth = gl_FragCoord.z;
	vec3 ndcPosCone = vec3(texCoord, coneDepth) * 2 - 1;
	vec4 invClipPosCone = inverseProjView * vec4(ndcPosCone, 1);
	vec3 coneWorldPos = invClipPosCone.xyz / invClipPosCone.w;

	// Trying out volumetric fog but not working :( 
	bool hitSurface = depth < coneDepth;
	vec3 targetPos = hitSurface ? worldPos : coneWorldPos;

	// Surface lighting
	vec3 incident = normalize(lightPos - targetPos);
	vec3 viewDir = normalize(cameraPos - targetPos);
	vec3 halfDir = normalize(incident + viewDir);

	// cos(theta), theta - angle between incident and light direction
	float theta = dot(incident, normalize(-lightDir)); // No light out of volume
	float epsilon = lightInnerCutoff - lightOuterCutoff;
	float intensity = clamp((theta - lightOuterCutoff) / epsilon, 0, 1); // 0 when outside light
	//intensity = pow(intensity, 0.75);
	intensity = smoothstep(0.0, 1.0, intensity);

	// Attenuation calculation
	float dist = length(lightPos - targetPos);
	float beamDist = min(dist, 20);

	float range = attenValues.x;
	float constant = attenValues.y;
	float linear = attenValues.z;
	float quadratic = attenValues.w;

	// weaken quadratic value so light is visible for longer distances
	float beamAtten = 1.0 / (constant + linear * beamDist + (quadratic * 0.2) * (beamDist * beamDist));
	float atten = 1.0 / (constant + linear * dist + (quadratic*0.1) * (dist * dist));

	vec3 surfaceColour = vec3(0);
	vec3 surfaceSpec = vec3(0);

	if (hitSurface) {
		vec4 normalData = texture(normTex, texCoord.xy);
		vec3 normal = normalize(normalData.xyz * 2 - 1);
		float roughness = normalData.a;
		// Roughness (0 = Glossy, 1 = Matte) -> Specular power (High = Glossy, Low = Matte)
		float shininess = mix(60, 1, roughness);

		float lambert = clamp(dot(incident, normal), 0, 1);
		float rFactor = clamp(dot(halfDir, normal), 0, 1);
		float specFactor = pow(rFactor, shininess);
		float specularIntensity = 1.0 - roughness;

		surfaceColour = lightDiffColour.rgb * intensity * atten * lambert;
		surfaceSpec = lightSpecColour.rgb * intensity * atten * specFactor * specularIntensity;
	}

	float fogDensity = 0.5; // Clear = 0, Thick = 1
	vec3 fogColour = lightDiffColour.rgb * intensity * beamAtten * fogDensity;
	
	//vec3 attenuated = lightColour.rgb * intensity;
	diffuseOutput = vec4(surfaceColour + fogColour, 1);
	specularOutput = vec4(surfaceSpec, 1);
}