#version 330 core

uniform sampler2D diffuseTex;
uniform sampler2D diffuseLight;
uniform sampler2D specularLight;

in Vertex { 
	vec2 texCoord;
} IN;

out vec4 fragColour;

void main() {
	vec4 diffuseSample = texture(diffuseTex, IN.texCoord);
	if (diffuseSample.a == 0.0)
		discard;

	vec3 diffuse = diffuseSample.rgb;
	vec3 light = texture(diffuseLight, IN.texCoord).rgb;
	vec3 specular = texture(specularLight, IN.texCoord).rgb;

	fragColour.xyz = diffuse * 0.1;
	fragColour.xyz += diffuse * light;
	fragColour.xyz += specular;
	fragColour.a = 1;
}