#version 330 core

layout (location = 0) in vec3 vertex_pos_modelspace;
layout (location = 1) in vec2 vertex_UV;
layout (location = 2) in vec3 vertex_normal_modelspace;

layout (location = 3) in vec3 instance_pos_worldspace;
layout (location = 4) in uint instance_block_type;
layout (location = 5) in uint instance_faces_mask;
// in int gl_VertexID;

// out vec2 fragment_UV;
// out vec3 fragment_pos_worldspace;
// out vec3 fragment_normal_worldspace;

out VS_OUT {
	bool discard_triangle;

	vec2 fragment_UV;
	vec3 fragment_pos_worldspace;
	vec3 fragment_normal_worldspace;
} vs_out;
// out vec4 gl_Position // Vertex model space position

uniform mat4 M;
uniform mat4 V;
uniform mat4 P;

void main()
{
	if (instance_block_type == 0u //) {
		|| (instance_faces_mask&(1u<<(gl_VertexID/6))) == 0u) {

		vs_out.fragment_pos_worldspace = vec3(0, 0, 2);
		vs_out.fragment_normal_worldspace = vec3(0, 0, -1);
		vs_out.fragment_UV = vec2(0, 0);

		gl_Position = vec4(0, 0, 2, 1);
		vs_out.discard_triangle = true;
		return;
	}

	vs_out.discard_triangle = false;

	vs_out.fragment_pos_worldspace = vec3(M * vec4(vertex_pos_modelspace, 1.0));
	vs_out.fragment_pos_worldspace += instance_pos_worldspace;

	vs_out.fragment_normal_worldspace
		= mat3(transpose(inverse(M))) * vertex_normal_modelspace;

	gl_Position = P * V * vec4(vs_out.fragment_pos_worldspace, 1.0);
	vs_out.fragment_UV = vertex_UV;
}
