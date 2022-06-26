#ifndef CONTROLS_HPP
#define CONTROLS_HPP

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

void computeMatricesFromInputs(GLint windowWidth, GLint windowHeight);
glm::mat4 getViewMatrix();
glm::mat4 getProjectionMatrix();

extern glm::vec3 position;
extern float horizontalAngle;
extern float verticalAngle;

#endif
