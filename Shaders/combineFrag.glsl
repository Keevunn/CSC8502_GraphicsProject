#version 330 core

uniform sampler2D diffuseTex;
uniform sampler2D emissiveTex;
uniform sampler2D diffuseLight;
uniform sampler2D specularLight;

in Vertex { 
	vec2 texCoord;
} IN;

out vec4 fragColour;

void main() {
	vec4 diffuseSample = texture(diffuseTex, IN.texCoord);
	vec3 emissive = texture(emissiveTex, IN.texCoord).rgb;

	// Skybox Check
	if (diffuseSample.a == 0.0) discard; 

	vec3 albedo = diffuseSample.rgb;
	float metallic = clamp((diffuseSample.a - 0.1)/0.9, 0.0, 1.0); // Restore 0.0 - 1.0 range

	vec3 light = texture(diffuseLight, IN.texCoord).rgb;
	vec3 specular = texture(specularLight, IN.texCoord).rgb;

	// Metal: coloured (albedo), Plastic: white
	vec3 specColour = mix(vec3(1), albedo, metallic);
	// Metal: black (no diffuse reflection)
	vec3 diffFactor = vec3(1 - metallic);

	fragColour.xyz = albedo * 0.1; // ambient
	fragColour.xyz += albedo * light * diffFactor; // diffuse
	fragColour.xyz += specular * specColour; // specular
	fragColour.xyz += emissive; // emissive
	fragColour.a = 1;
}