#version 410 core

layout(triangles, equal_spacing, ccw) in;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projMatrix;

uniform float time;
uniform vec4 colour;

in vec3 worldPos_ESin[];
in vec2 texCoord_ESin[];
in vec3 normal_ESin[];

out Vertex {
	vec4 colour;
	vec2 texCoord;
	vec3 normal;
	vec3 worldPos;
} OUT;

// No waves: 0, 1: rough waves 
# define WAVE_STRENGTH 0.2

struct Wave {
	float freq;
	float speed;
};

// x: wave height, y: dx for normals, z: dz for normals
vec3 calculateSurface(float x, float z) {
    float y = 0.0;
	float dx = 0;
	float dz = 0;

	const int NUM_WAVES = 3;
	float freq = 0;
	float speed = 0;

	//y += (sin(x * 1.00 * WAVE_STRENGTH + time * 1.00) + sin(x * 1.34 * WAVE_STRENGTH + time * 1.5) + sin(x * 2.03 * WAVE_STRENGTH + time * 0.40)) / 3.0;
    //y += (sin(z * 0.20 * WAVE_STRENGTH + time * 0.57) + sin(z * 1.80 * WAVE_STRENGTH + time * 2.5) + sin(z * 2.80 * WAVE_STRENGTH + time * 1.48)) / 3.0;
	Wave xWaves[NUM_WAVES] = Wave[]( Wave(1.00 * WAVE_STRENGTH, 1.00), Wave(1.34 * WAVE_STRENGTH, 1.50), Wave(2.03 * WAVE_STRENGTH, 0.40) );
	Wave zWaves[NUM_WAVES] = Wave[]( Wave(0.20 * WAVE_STRENGTH, 0.57), Wave(1.80 * WAVE_STRENGTH, 2.81), Wave(2.80 * WAVE_STRENGTH, 1.48) );

	// Calculate X waves height and X gradient
	for (int i = 0; i < NUM_WAVES; ++i) {
		float freq = xWaves[i].freq;
		float speed = xWaves[i].speed;
		y += sin(x * freq + time * speed);
		dx += freq * cos(x * freq + time * speed);
	}

	// Calculate Z waves height and Z gradient
	for (int i = 0; i < NUM_WAVES; ++i) {
		float freq = zWaves[i].freq;
		float speed = zWaves[i].speed;
		y += sin(z * freq + time * speed);
		dz += freq * cos(z * freq + time * speed);
	}

    return vec3(y / NUM_WAVES, dx / NUM_WAVES, dz / NUM_WAVES);
}

// Interpolate vectors based on barycentric coords (gl_TessCoord)
vec3 Interpolate3D(vec3 v0, vec3 v1, vec3 v2) { return gl_TessCoord.x * v0 + gl_TessCoord.y * v1 + gl_TessCoord.z * v2; }
vec2 Interpolate2D(vec2 v0, vec2 v1, vec2 v2) { return gl_TessCoord.x * v0 + gl_TessCoord.y * v1 + gl_TessCoord.z * v2; }

void main() {
	vec2 texCoord = Interpolate2D(texCoord_ESin[0], texCoord_ESin[1], texCoord_ESin[2]);
	OUT.texCoord = texCoord;
	OUT.colour = colour;

	vec3 pos = Interpolate3D(worldPos_ESin[0], worldPos_ESin[1], worldPos_ESin[2]);
	vec3 waveData = calculateSurface(pos.x, pos.z);
	vec3 offsetData = calculateSurface(115.2f, 66.8f);
	float height = waveData.x;
	float dx = waveData.y;
	float dz = waveData.z;
	pos.y += height;
	pos.y -= offsetData.x;

	// Cross product of tangent vectors (1, dx, 0) and (0, dz, 1) is (-dx, 1, -dz)
	vec3 normal = normalize(vec3(-dx, 1, -dz));
	mat3 normalMatrix = transpose(inverse(mat3(modelMatrix)));
	OUT.normal = normalize(normalMatrix * normal);

	vec4 worldPos = modelMatrix * vec4(pos, 1);
	OUT.worldPos = worldPos.xyz;

	mat4 projView = projMatrix * viewMatrix;
	gl_Position = projView * worldPos;
}