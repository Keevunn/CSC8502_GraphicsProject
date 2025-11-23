#version 330 core

uniform sampler2D diffuseTex;
uniform sampler2D noiseTex;

in Vertex {
	vec2 texCoord;
} IN;

out vec4 fragColour;

void main(void) {
	vec4 colour = texture(diffuseTex, IN.texCoord);
	float noiseValue = texture(noiseTex, IN.texCoord * 0.125).r; // different scale so it doesn't line up with diffuseTex
	
	//mix(original, darker, strength)
	vec3 finalColour = mix(colour.rgb, colour.rbg * 0.4, noiseValue); 

	fragColour = vec4(finalColour, 1);
}