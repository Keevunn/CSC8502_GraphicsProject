#version 330 core

uniform sampler2D opacityTex;
uniform samplerCube cubeTex; // Might not reflect sky for cartoony look

uniform vec3 cameraPos;
uniform float time;

in Vertex {
	vec4 colour;
	vec2 texCoord;
	vec3 normal;
	vec3 worldPos;
} IN;

// 0: Colour, 1: Normal, 2: Emissive
out vec4 fragColour[3];
// Metallic in colour alpha: 0.1 (so fragment not discarded)
// Roughness in normal alpha: 0

# define TEX_SCALE 20.0

void main(void) {
	// 0.05 - speed of moving animation
	vec2 newTexCoord = IN.texCoord * TEX_SCALE + vec2(time * -0.05); // water tex repeats

	newTexCoord.y += 0.61 * (sin(newTexCoord.x * 5.2 + time * 0.17) + sin(newTexCoord.x * 2.75 + time * 0.68) + sin(newTexCoord.x * 3.16 + time * 1.02)) / 3.0;
	newTexCoord.x += 0.35 * (sin(newTexCoord.y * 1.5 + time * 0.43) + sin(newTexCoord.y * 4.26 + time * 0.06) + sin(newTexCoord.y * 2.29 + time * 0.82)) / 3.0;
	newTexCoord.y += 0.12 * (sin(newTexCoord.x * 6.2 + time * 0.77) + sin(newTexCoord.x * 1.90 + time * 0.83) + sin(newTexCoord.x * 6.50 + time * 0.64)) / 3.0;

	vec4 ripples = texture(opacityTex, newTexCoord);
	vec4 shadowRipples = texture(opacityTex, newTexCoord + vec2(0.2)); // Slightly translated so it doesn't line up

	vec3 viewDir = normalize(cameraPos - IN.worldPos); // frag -> cam
	vec3 reflectDir = reflect(-viewDir, normalize(IN.normal)); // reflect viewDir around normal
	vec4 reflectTex = texture(cubeTex, reflectDir);

	// Colour
	vec3 ripplesColour = vec3(ripples.a * 0.9 - shadowRipples.a * 0.02);
	vec4 waterColour = IN.colour + vec4(ripplesColour, 1.0);
	vec4 blendedColour = mix(waterColour, reflectTex, 0.25);
	fragColour[0] = vec4(blendedColour.rgb, 0.1);
	
	// Normal
	vec3 normal = normalize(IN.normal);
	fragColour[1] = vec4(normal * 0.5 + 0.5, 0);

	// Emissive (no emissive lights)
	fragColour[2] = vec4(0);
}