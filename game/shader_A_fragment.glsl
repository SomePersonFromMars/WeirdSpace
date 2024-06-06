// Copyright (C) 2024, Kacper Orszulak
// GNU General Public License v3.0+ (see LICENSE.txt or https://www.gnu.org/licenses/gpl-3.0.txt)

#version 330 core

out vec4 color;

in vec2 fragment_UV;
in vec3 fragment_pos_worldspace;
in vec3 fragment_normal_worldspace;

uniform sampler2D texture_sampler;

// Common fragment shader fields
uniform vec3 camera_pos_worldspace;
uniform vec3 light_pos_worldspace;
uniform vec3 sun_direction_worldspace;
uniform vec3 light_color;
uniform vec3 fog_color;

// Credits: https://iquilezles.org/articles/fog/
vec3 apply_fog(
        in vec3 col, // color of pixel
        in float d, // distance to point
        in vec3 point_to_camera
    ) {
    const float b = 0.002;

    float fog_amount = 1.0 - exp(-d*b);
    float sun_amount = max(dot(point_to_camera, sun_direction_worldspace), 0.0);
    vec3 modified_fog_color = mix(
            fog_color,
            // vec3(0.5, 0.6, 0.7), // blue
            vec3(1.0, 0.9, 0.7), // yellow
            // vec3(1.0),
            pow(sun_amount, 2.0)
            );
    return mix(col, modified_fog_color, fog_amount);
}


void main()
{
	vec4 objectColor = texture( texture_sampler, fragment_UV ).rgba;
	// vec4 objectColor = vec4(1, 1, 1, 1);
	if (objectColor.a < 0.5)
		discard;

	// ambient
	float ambientStrength = 0.3;
	vec3 ambient = ambientStrength * light_color;

	// diffuse
	vec3 norm = normalize(fragment_normal_worldspace);
	vec3 lightDir = normalize(light_pos_worldspace - fragment_pos_worldspace);
	// vec3 lightDir = normalize(-vec3(3, -1, -3));
	// vec3 lightDir = normalize(-vec3(1, -1, 1));
	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = diff * light_color;
	// float dist = length(light_pos_worldspace - fragment_pos_worldspace);
	// vec3 diffuse = diff * light_color * 100 / dist / dist;

	vec3 result = (ambient + diffuse) * objectColor.rgb;

    // fog
    vec3 frag_to_camera = camera_pos_worldspace - fragment_pos_worldspace;
    float cam_frag_dist = length(frag_to_camera);
    result = apply_fog(result, cam_frag_dist, normalize(frag_to_camera));

	color = vec4(result, 1.0);

	// // Disable the light
	// color = vec4(objectColor, 1.0);
}
