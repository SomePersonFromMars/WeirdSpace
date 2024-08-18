// Copyright (C) 2024, Kacper Orszulak
// GNU General Public License v3.0+ (see LICENSE.txt or https://www.gnu.org/licenses/gpl-3.0.txt)

#version 330

layout (location = 0) in vec3 vertex_pos_modelspace;
layout (location = 1) in vec2 vertex_UV;
uniform mat4 MVP_matrix;

out vec2 fragment_UV;
// out vec4 gl_Position

void main() {
	fragment_UV = vertex_UV;
	gl_Position = MVP_matrix * vec4(vertex_pos_modelspace, 1.0);
}
