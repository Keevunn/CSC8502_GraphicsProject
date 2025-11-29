#version 330 core

uniform sampler2D diffuseTex;
uniform sampler2D emissiveTex;

uniform sampler2D diffuseLight;
uniform sampler2D specularLight;

uniform samplerCube skyboxTex;
uniform sampler2D normTex;

// For fog calculations
uniform sampler2D depthTex;
uniform vec3 cameraPos;
uniform mat4 inverseProjView;

in Vertex { 
	vec2 texCoord;
} IN;

out vec4 fragColour;

void main() {
	vec4 diffuseSample = texture(diffuseTex, IN.texCoord);
	vec3 emissive = texture(emissiveTex, IN.texCoord).rgb;
	// Skybox Check
	if (diffuseSample.a == 0.0) discard; 

	float depth = texture(depthTex, IN.texCoord).r;
	vec3 ndcPos = vec3(IN.texCoord, depth) * 2 - 1;
	vec4 invClipPos = inverseProjView * vec4(ndcPos, 1);
	vec3 worldPos = invClipPos.xyz / invClipPos.w;

	vec3 albedo = diffuseSample.rgb;
	float metallic = clamp((diffuseSample.a - 0.1)/0.9, 0.0, 1.0); // Restore 0.0 - 1.0 range

	vec3 light = texture(diffuseLight, IN.texCoord).rgb;
	vec3 specular = texture(specularLight, IN.texCoord).rgb;

	// Metal: coloured (albedo), Plastic: white
	vec3 specColour = mix(vec3(1), albedo, metallic);
	// Metal: black (no diffuse reflection)
	vec3 diffFactor = vec3(1 - metallic);

	// Skybox reflection
	vec4 normalData = texture(normTex, IN.texCoord);
	vec3 normal = normalize(normalData.xyz * 2 - 1);
	vec4 diffuse = texture(diffuseTex, IN.texCoord);
	vec3 viewDir = normalize(cameraPos - worldPos); // frag -> cam

	vec3 reflectDir = reflect(-viewDir, normalize(normal)); // reflect viewDir around normal
	vec4 reflectTex = texture(skyboxTex, reflectDir);

	vec3 finalColour = albedo * 0.1; // ambient
	finalColour += albedo * light * diffFactor; // diffuse
	finalColour += specular * specColour; // specular
	finalColour += reflectTex.rgb * specColour * metallic;
	finalColour += emissive; // emissive

	float dist = length(worldPos - cameraPos);

	// Start fading distance: 80m
	// Opaque distance: 180m 
	float fogStart = 25;
	float fogEnd = 100;
	float fogFactor = clamp((dist - fogStart) / (fogEnd - fogStart), 0, 1);

	vec3 fogColour = vec3(0.2, 0.2, 0.2);

	fragColour.xyz = mix(finalColour, fogColour, fogFactor);
	fragColour.a = 1;
}