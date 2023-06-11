#version 450 core

layout (location = 0) in vec2 pos;
layout (location = 1) in uint type;

const vec3[] COLORS = vec3[](
	vec3(0.0,0.0,0.0),
	vec3(0.227,0.588,0.282), // LAND
	vec3(0.467,0.769,0.867) // WATER
);

out VS_OUT {
	vec4 color;
} vs_out;

void main(void) {
	// vec4[3] positions = vec4[3](
	// 	vec4(-0.5, -0.5, 0.5, 1.0),
	// 	vec4( 0.0,  0.5, 0.5, 1.0),
	// 	vec4( 0.5, -0.5, 0.5, 1.0)
	// );
	// gl_Position = positions[gl_VertexID];
	gl_Position = vec4(pos, 0.0, 1.0);
	// vs_out.color = vec4(0.5);
	vs_out.color = vec4(COLORS[type], 1.0);
	// vs_out.color = vec4(vec3(0.227,0.588,0.282), 1.0);
}
