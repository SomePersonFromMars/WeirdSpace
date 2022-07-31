#version 330 core

#define BLOCK_VERTICES_CNT (6*2*3)

layout (points) in;
layout (triangle_strip, max_vertices=/*BLOCK_VERTICES_CNT*/36) out;

in VS_OUT {
	int block_type;
} gs_in[];

out vec3 fragment_pos_worldspace;
// out vec4 gl_Position; // Vertex clip space position
out vec2 fragment_UV;
out vec3 fragment_normal_worldspace;

layout (std140) uniform block_model {
	vec4 vertices_positions    [BLOCK_VERTICES_CNT];
	vec4 vertices_uvs_combined [BLOCK_VERTICES_CNT*1];
	vec4 vertices_normals      [BLOCK_VERTICES_CNT];
};

uniform mat4 M;
uniform mat4 V;
uniform mat4 P;

vec4 stripped_positions[10];

void main() {
	stripped_positions[0] = vec4(0, 0, 0, 0);
	stripped_positions[1] = vec4(1, 0, 0, 0);
	stripped_positions[2] = vec4(0, 1, 0, 0);
	stripped_positions[3] = vec4(1, 1, 0, 0);
	stripped_positions[4] = vec4(0, 1, 1, 0);
	stripped_positions[5] = vec4(1, 1, 1, 0);
	stripped_positions[6] = vec4(0, 0, 0, 0);
	stripped_positions[7] = vec4(0, 0, 0, 0);
	stripped_positions[8] = vec4(0, 0, 0, 0);
	stripped_positions[9] = vec4(0, 0, 0, 0);

	int block_type = gs_in[0].block_type;

	if (block_type == 0) // 0 is block_type::none
		return;

	vec4 pos = gl_in[0].gl_Position;
	int uvs_off = BLOCK_VERTICES_CNT * (block_type-1);	// -1 since we omit
														// block_type::none
	mat4 PV = P*V;
	mat3 normal_matrix = mat3(transpose(inverse(M)));

	// for (int i = 0; i < 6; ++i) {
	// 	fragment_pos_worldspace = vec4(M * (pos + stripped_positions[i])).xyz;
	// 	gl_Position = PV*vec4(fragment_pos_worldspace, 1);
	// 	// fragment_pos_worldspace = vec4((pos + stripped_positions[i])).xyz;
	// 	// gl_Position = vec4(fragment_pos_worldspace, 1);

	// 	fragment_UV.x = vertices_uvs_combined[uvs_off + i].x;
	// 	fragment_UV.y = vertices_uvs_combined[uvs_off + i].y;

	// 	fragment_normal_worldspace
	// 		= normal_matrix * vertices_normals[i].xyz;

	// 	EmitVertex();
	// }
	// EndPrimitive();
	// return;

	for (int i = 0; i < 3; ++i) {
		fragment_pos_worldspace = vec4(M * (pos + vertices_positions[i])).xyz;
		gl_Position = PV*vec4(fragment_pos_worldspace, 1);

		fragment_UV.x = vertices_uvs_combined[uvs_off + i].x;
		fragment_UV.y = vertices_uvs_combined[uvs_off + i].y;

		fragment_normal_worldspace
			= normal_matrix * vertices_normals[i].xyz;

		EmitVertex();
		if (i % 3 == 2)
			EndPrimitive();
	}
}
