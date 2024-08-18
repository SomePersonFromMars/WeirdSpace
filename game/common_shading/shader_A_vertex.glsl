// Copyright (C) 2024, Kacper Orszulak
// GNU General Public License v3.0+ (see LICENSE.txt or https://www.gnu.org/licenses/gpl-3.0.txt)

#version 330 core

layout (location = 0) in vec3 vertex_pos_modelspace;
layout (location = 1) in vec2 vertex_UV;
layout (location = 2) in vec3 vertex_normal_modelspace;
layout (location = 3) in vec3 instance_pos_worldspace;

out vec2 fragment_UV;
out vec3 fragment_pos_worldspace;
out vec3 fragment_normal_worldspace;

uniform mat4 M;
uniform mat4 V;
uniform mat4 P;

void main()
{
	fragment_pos_worldspace = vec3(M * vec4(vertex_pos_modelspace, 1.0));
	fragment_pos_worldspace += instance_pos_worldspace;

	fragment_normal_worldspace
		= mat3(transpose(inverse(M))) * vertex_normal_modelspace;

	gl_Position = P * V * vec4(fragment_pos_worldspace, 1.0);
	fragment_UV = vertex_UV;
}
