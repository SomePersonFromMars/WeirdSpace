#version 330 core

#define ATTEMPT_B
#ifndef ATTEMPT_B

// Interpolated values from the vertex shaders
in vec2 UV;
in vec3 Position_worldspace;
in vec3 Normal_cameraspace;
in vec3 EyeDirection_cameraspace;
in vec3 LightDirection_cameraspace;

// Ouput data
out vec3 color;

// Values that stay constant for the whole mesh.
uniform sampler2D myTextureSampler;
uniform mat4 MVP;
uniform vec3 LightPosition_worldspace;

void main(){

	// Light emission properties
	// You probably want to put them as uniforms
	vec3 LightColor = vec3(1,1,1);
	float LightPower = 100.0f;

	// Material properties
	vec3 MaterialDiffuseColor = texture( myTextureSampler, UV ).rgb;
	// vec3 MaterialAmbientColor = vec3(0.1,0.1,0.1) * MaterialDiffuseColor;
	vec3 MaterialAmbientColor = 0.1 * MaterialDiffuseColor;
	vec3 MaterialSpecularColor = 0.3 * vec3(1,1,1);

	// Distance to the light
	float distance = length( LightPosition_worldspace - Position_worldspace );

	// Normal of the computed fragment, in camera space
	vec3 n = normalize( Normal_cameraspace );
	// Direction of the light (from the fragment to the light)
	vec3 l = normalize( LightDirection_cameraspace );
	// Cosine of the angle between the normal and the light direction,
	// clamped above 0
	//  - light is at the vertical of the triangle -> 1
	//  - light is perpendicular to the triangle -> 0
	//  - light is behind the triangle -> 0
	float cosTheta = clamp( dot( n,l ), 0,1 );

	// Eye vector (towards the camera)
	vec3 E = normalize(EyeDirection_cameraspace);
	// Direction in which the triangle reflects the light
	vec3 R = reflect(-l,n);
	// Cosine of the angle between the Eye vector and the Reflect vector,
	// clamped to 0
	//  - Looking into the reflection -> 1
	//  - Looking elsewhere -> < 1
	float cosAlpha = clamp( dot( E,R ), 0,1 );

	color =
		// Ambient : simulates indirect lighting
		MaterialAmbientColor +
		// Diffuse : "color" of the object
		// MaterialDiffuseColor * LightColor * LightPower * cosTheta / (distance*distance) +
		MaterialDiffuseColor * LightColor * cosTheta +
		// MaterialDiffuseColor * clamp(LightColor * LightPower * cosTheta / (distance*distance), 0, 0.95) +
		// MaterialDiffuseColor * sqrt(LightColor * LightPower * cosTheta / (distance*distance)) +
		// Specular : reflective highlight, like a mirror
		0;
		// MaterialSpecularColor * LightColor * LightPower * pow(cosAlpha,5) / (distance*distance);

	// color = MaterialDiffuseColor;
}

#else

out vec4 color;

in vec2 UV;
in vec3 Position_worldspace;
in vec3 Normal_worldspace;

uniform vec3 LightPosition_worldspace;
uniform vec3 LightColor;
// uniform vec3 viewPos;
// uniform vec3 lightColor;
// uniform vec3 objectColor;
uniform sampler2D myTextureSampler;

void main()
{
	vec3 objectColor = texture( myTextureSampler, UV ).rgb;
	// vec3 lightColor = vec3(1, 1, 1);
	vec3 lightColor = LightColor;

	// ambient
	float ambientStrength = 0.3;
	vec3 ambient = ambientStrength * lightColor;

	// diffuse
	vec3 norm = normalize(Normal_worldspace);
	vec3 lightDir = normalize(LightPosition_worldspace - Position_worldspace);
	// vec3 lightDir = normalize(-vec3(3, -1, -3));
	// vec3 lightDir = normalize(-vec3(1, -1, 1));
	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = diff * lightColor;
	// float dist = length(LightPosition_worldspace - Position_worldspace);
	// vec3 diffuse = diff * lightColor * 100 / dist / dist;

	vec3 result = (ambient + diffuse) * objectColor;
	color = vec4(result, 1.0);

	// // Disable the light
	// color = vec4(objectColor, 1.0);
}

#endif
