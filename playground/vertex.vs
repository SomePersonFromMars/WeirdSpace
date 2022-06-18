#version 330 core

// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec3 vertexPosition_modelspace;
layout(location = 1) in vec3 vertexColor;

out vec3 fragmentColor;

uniform mat4 MVP;

void main(){
	fragmentColor = vertexColor;

	gl_Position.xyz = vertexPosition_modelspace;
	gl_Position.w = 1.0;
	gl_Position = MVP * gl_Position;
}
