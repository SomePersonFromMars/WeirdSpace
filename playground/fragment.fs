#version 330 core

uniform sampler2D myTextureSampler;

// in vec3 fragmentColor;
in vec2 fragmentUV;

// Ouput data
out vec3 color;

void main()
{
	// Output color = red
	// color = vec3(1,0,0);

	// color = fragmentColor;

	color = texture(myTextureSampler, fragmentUV).rgb;
}
