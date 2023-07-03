#version 450 core

layout (location = 0) in vec2 pos;
layout (location = 1) in uint type;

uniform vec2 space_max;

const vec3[] COLORS = vec3[](
	vec3(0.0, 0.0, 0.0),
	// vec3(0.227,0.588,0.282), // LAND
	// vec3(0.467,0.769,0.867) // WATER
	vec3(1.0, 1.0, 1.0),
	vec3(0.0, 0.0, 0.0)
);

out VS_OUT {
	vec4 color;
} vs_out;

void main(void) {
	vec2 P = pos;
	P.x -= space_max.x / 3.0;
	P /= space_max;
	P = 2.0*P-1.0;
	P.x += float(gl_InstanceID) * 2.0 / 3.0;
	gl_Position = vec4(P, 0.0, 1.0);

	vs_out.color = vec4(COLORS[type], 1.0);
}
