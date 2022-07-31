#version 330 core

layout (triangles) in;
layout (triangle_strip, max_vertices=3) out;

in VS_OUT {
	bool discard_triangle;

	vec2 fragment_UV;
	vec3 fragment_pos_worldspace;
	vec3 fragment_normal_worldspace;
} gs_in[];

out vec3 fragment_pos_worldspace;
// out vec4 gl_Position; // Vertex clip space position
out vec2 fragment_UV;
out vec3 fragment_normal_worldspace;

void main() {
	if (gs_in[0].discard_triangle)
		return;

	for (int i = 0; i < 3; ++i) {
		fragment_UV = gs_in[i]. fragment_UV;
		fragment_pos_worldspace = gs_in[i]. fragment_pos_worldspace;
		fragment_normal_worldspace = gs_in[i]. fragment_normal_worldspace;
		gl_Position = gl_in[i].gl_Position;
		EmitVertex();
	}
	EndPrimitive();
}
