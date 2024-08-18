// Copyright (C) 2024, Kacper Orszulak
// GNU General Public License v3.0+ (see LICENSE.txt or https://www.gnu.org/licenses/gpl-3.0.txt)

#version 330

in vec2 fragment_UV;
out vec3 color;

uniform sampler2D texture_sampler;

void main() {
	color = texture(texture_sampler, fragment_UV).rgb;
}
