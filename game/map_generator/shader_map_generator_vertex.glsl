#version 450 core

layout (location = 0) in vec2 pos;
// layout (location = 1) in uint type;
layout (location = 1) in float elevation;

uniform vec2 space_max;
uniform bool triple_map_size;

// const vec3[] COLORS = vec3[](
// 	vec3(0.0, 0.0, 0.0),
// 	// vec3(0.227,0.588,0.282), // LAND
// 	// vec3(0.467,0.769,0.867) // WATER
// 	vec3(1.0, 1.0, 1.0),
// 	vec3(0.0, 0.0, 0.0)
// );

// const float[] ELEVATIONS = float[](
// 	0.0, // NONE
// 	1.0, // LAND
// 	0.0  // WATER
// );

out VS_OUT {
	float elevation;
	vec2 pos;
} vs_out;

void main(void) {
	const vec2 real_space_max = vec2(space_max.x*3.0, space_max.y);
	vec2 P = pos;
	P.x -= space_max.x;
	P /= real_space_max;
	P.x += float(gl_InstanceID) * 1.0 / 3.0;
	P = 2.0*P-1.0;
	if (!triple_map_size) {
		P.x *= 3.0;
	}
	gl_Position = vec4(P, 0.0, 1.0);

	// vs_out.color = vec4(COLORS[type], 1.0);
	// vs_out.elevation = ELEVATIONS[type];
	vs_out.elevation = elevation;

	vs_out.pos = pos;
}
