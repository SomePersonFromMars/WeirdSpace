// Copyright (C) 2024, Kacper Orszulak
// GNU General Public License v3.0+ (see LICENSE.txt or https://www.gnu.org/licenses/gpl-3.0.txt)

#version 450 core

layout (location = 0) in vec2 pos;
// layout (location = 1) in uint type;
layout (location = 1) in float elevation;

uniform vec2 space_max;
uniform bool triple_map_size;

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

	vs_out.elevation = elevation;

	vs_out.pos = pos;
}
