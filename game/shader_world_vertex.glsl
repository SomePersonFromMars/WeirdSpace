#version 330 core

layout (location = 0) in int block_type;
// in int gl_VertexID;

out VS_OUT {
	int block_type;
} vs_out;
// out vec4 gl_Position // Vertex model space position

uniform ivec3 chunk_dim; // Chunk dimensions

void main()
{
	int id = gl_VertexID;
	ivec3 ipos;
	ipos.z = id % chunk_dim.z;
	id -= ipos.z;
	id /= chunk_dim.z;

	ipos.y = id % chunk_dim.y;
	id -= ipos.y;
	id /= chunk_dim.y;

	ipos.x = id;

	gl_Position = vec4(ipos.x, ipos.y, ipos.z, 1);
	vs_out.block_type = block_type;
}
