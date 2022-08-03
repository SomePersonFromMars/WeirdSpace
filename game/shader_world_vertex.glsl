#version 330 core

// There is no per vertex data, but gl_VertexID
// in int gl_VertexID;

// Instance data
layout (location = 0) in vec3 instance_pos_worldspace;	// Current block
														// right-bottom-front
														// corner world space
														// position
layout (location = 1) in uint instance_block_type; 	// Value 0 is useful for
													// discarding a face
layout (location = 2) in uint instance_face_type; // [0, 5]

out vec2 fragment_UV;
out vec3 fragment_pos_worldspace;
out vec3 fragment_normal_worldspace;
// out vec4 gl_Position // Vertex clip space position

#define BLOCK_VERTICES_CNT (6u*2u*3u)
layout (std140) uniform block_model {
	vec4 vertices_positions    [BLOCK_VERTICES_CNT];
	vec4 vertices_uvs_combined [BLOCK_VERTICES_CNT*2u];	// Where 1 is
														// block_type::cnt-1
	vec4 vertices_normals      [BLOCK_VERTICES_CNT];
};

uniform mat4 M;
uniform mat4 V;
uniform mat4 P;

void main()
{
	if (instance_block_type == 0u) {
		fragment_pos_worldspace = vec3(0, 0, 2);
		fragment_normal_worldspace = vec3(0, 0, -1);
		fragment_UV = vec2(0, 0);
		gl_Position = vec4(0, 0, 2, 1);
		return;
	}

	uint id = instance_face_type*6u + uint(gl_VertexID);
	uint uvs_off = BLOCK_VERTICES_CNT * (instance_block_type-1u);

	vec3 vertex_pos_modelspace = vertices_positions[id].xyz;
	vec2 vertex_UV = vertices_uvs_combined[uvs_off + id].xy;
	vec3 vertex_normal_modelspace = vertices_normals[id].xyz;

	fragment_pos_worldspace = vec3(M * vec4(vertex_pos_modelspace, 1.0));
	fragment_pos_worldspace += instance_pos_worldspace;

	fragment_normal_worldspace
		= mat3(transpose(inverse(M))) * vertex_normal_modelspace;

	gl_Position = P * V * vec4(fragment_pos_worldspace, 1.0);
	fragment_UV = vertex_UV;
}
