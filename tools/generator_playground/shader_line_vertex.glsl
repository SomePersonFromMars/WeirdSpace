#version 330

layout (location = 0) in vec3 vertex_pos_modelspace;
uniform mat4 MVP_matrix;

out vec2 fragment_UV;
// out vec4 gl_Position

void main() {
	gl_Position = MVP_matrix * vec4(vertex_pos_modelspace, 1.0);
}
