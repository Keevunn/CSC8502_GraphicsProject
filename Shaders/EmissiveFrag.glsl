#version 330 core

uniform sampler2D emissiveTex;
uniform sampler2D alphaTex;

uniform float emissionIntensity; // Controls brightness
uniform float time;

in Vertex {
	vec2 texCoord;
} IN;

out vec4 fragColour;

void main() {
	float opacity = texture(alphaTex, IN.texCoord).r;
	if (opacity < 0.1) discard;

	// Range: 0 - 1
	float pulse = (sin(time * 3.0) * 0.5) + 0.5;

	vec3 glow = texture(emissiveTex, IN.texCoord).rgb;
	fragColour = vec4(glow * emissionIntensity * pulse, 1);
}