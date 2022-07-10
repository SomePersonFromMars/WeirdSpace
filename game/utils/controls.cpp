// Include GLFW
#include <GLFW/glfw3.h>
extern GLFWwindow* window; // The "extern" keyword here is to access the variable "window" declared in tutorialXXX.cpp. This is a hack to keep the tutorials simple. Please avoid this.

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include "controls.hpp"

glm::mat4 ViewMatrix;
glm::mat4 ProjectionMatrix;

glm::mat4 getViewMatrix(){
	return ViewMatrix;
}
glm::mat4 getProjectionMatrix(){
	return ProjectionMatrix;
}


// Initial position
glm::vec3 position = glm::vec3( 0, 1, 5 );
// Initial horizontal angle
float horizontalAngle = 3.0f;
// Initial vertical angle
float verticalAngle = 6.0f;

// Initial Field of View
float initialFoV = 120.0f;

constexpr float speed_normal = 8.0f; // 3 units / second
constexpr float speed_accelerated = 64.0f; // 3 units / second
constexpr float rotate_speed_normal = 2.0f;
constexpr float rotate_speed_slowed = 0.5f;
float mouseSpeed = 0.005f;

void computeMatricesFromInputs(GLint windowWidth, GLint windowHeight ){

	// glfwGetTime is called only once, the first time this function is called
	static double lastTime = glfwGetTime();

	// Compute time difference between current and last frame
	double currentTime = glfwGetTime();
	const float deltaTime = float(currentTime - lastTime);

	// Get mouse position
	double xpos, ypos;
	// glfwGetCursorPos(window, &xpos, &ypos);

	// Reset mouse position for next frame
	// glfwSetCursorPos(window, windowHeight/2, windowWidth/2);

	// Compute new orientation
	// horizontalAngle += mouseSpeed * float(windowHeight/2 - xpos );
	// verticalAngle   += mouseSpeed * float( windowWidth/2 - ypos );

	// Speed acceleration and rotation slowing down
	float speed = speed_normal;
	float rotate_speed = rotate_speed_normal;
	if (glfwGetKey( window, GLFW_KEY_LEFT_SHIFT ) == GLFW_PRESS){
		speed = speed_accelerated;
		rotate_speed = rotate_speed_slowed;
	}

	const float rotate_angle = rotate_speed * deltaTime;
	if (glfwGetKey( window, GLFW_KEY_UP ) == GLFW_PRESS) {
		verticalAngle += rotate_angle;
	}
	if (glfwGetKey( window, GLFW_KEY_DOWN ) == GLFW_PRESS) {
		verticalAngle -= rotate_angle;
	}
	if (glfwGetKey( window, GLFW_KEY_LEFT ) == GLFW_PRESS) {
		horizontalAngle += rotate_angle;
	}
	if (glfwGetKey( window, GLFW_KEY_RIGHT ) == GLFW_PRESS) {
		horizontalAngle -= rotate_angle;
	}

	// Direction : Spherical coordinates to Cartesian coordinates conversion
	glm::vec3 direction(
			cos(verticalAngle) * sin(horizontalAngle),
			sin(verticalAngle),
			cos(verticalAngle) * cos(horizontalAngle)
			);

	// Right vector
	glm::vec3 right = glm::vec3(
			sin(horizontalAngle - 3.14f/2.0f),
			0,
			cos(horizontalAngle - 3.14f/2.0f)
			);

	// Up vector
	glm::vec3 up = glm::cross( right, direction );

	// Move forward
	if (glfwGetKey( window, GLFW_KEY_W ) == GLFW_PRESS){
		position += direction * deltaTime * speed;
	}
	// Move backward
	if (glfwGetKey( window, GLFW_KEY_S ) == GLFW_PRESS){
		position -= direction * deltaTime * speed;
	}
	// Strafe right
	if (glfwGetKey( window, GLFW_KEY_D ) == GLFW_PRESS){
		position += right * deltaTime * speed;
	}
	// Strafe left
	if (glfwGetKey( window, GLFW_KEY_A ) == GLFW_PRESS){
		position -= right * deltaTime * speed;
	}

	float FoV = initialFoV;// - 5 * glfwGetMouseWheel(); // Now GLFW 3 requires setting up a callback for this. It's a bit too complicated for this beginner's tutorial, so it's disabled instead.

	// Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
	// ProjectionMatrix = glm::perspective(glm::radians(FoV), 4.0f / 3.0f, 0.1f, 100.0f);
	ProjectionMatrix = glm::perspective(glm::radians(FoV), (float)windowWidth / (float)windowHeight, 0.1f, 100.0f);
	// Camera matrix
	ViewMatrix       = glm::lookAt(
			position,           // Camera is here
			position+direction, // and looks here : at the same position, plus "direction"
			up                  // Head is up (set to 0,-1,0 to look upside-down)
			);

	// For the next frame, the "last time" will be "now"
	lastTime = currentTime;
}
