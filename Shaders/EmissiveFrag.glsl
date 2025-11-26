#version 330 core

uniform sampler2D emissiveTex;
uniform sampler2D alphaTex;

in Vertex {
	vec2 texCoord;
} IN;

out vec4 fragColour;

void main() {
	float opacity = texture(alphaTex, IN.texCoord).r;
	if (opacity < 0.1) discard;

	vec3 glow = texture(emissiveTex, IN.texCoord).rgb;
	fragColour = vec4(glow, 1);
}