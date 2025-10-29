#version 330 core // Version preprocessor definition

in vec3 position;
in vec4 colour;

// interface block
out Vertex {
	vec4 colour;
} OUT;

void main(void) {
	gl_Position = vec4(position, 1.0);
	OUT.colour = colour;
}