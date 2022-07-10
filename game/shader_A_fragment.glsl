#version 330 core

out vec4 color;

in vec2 fragment_UV;
in vec3 fragment_pos_worldspace;
in vec3 fragment_normal_worldspace;

uniform vec3 light_pos_worldspace;
uniform vec3 light_color;
uniform sampler2D texture_sampler;

void main()
{
	vec4 objectColor = texture( texture_sampler, fragment_UV ).rgba;
	if (objectColor.a == 0.0)
		discard;

	// light_color = vec3(1, 1, 1);

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
	color = vec4(result, 1.0);

	// // Disable the light
	// color = vec4(objectColor, 1.0);
}
