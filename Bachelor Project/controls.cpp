// Include GLFW
#include <GLFW/glfw3.h>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
using namespace glm;
#include "controls.h"
#include <iostream>

#define PI 3.14159265f
glm::mat4 ViewMatrix;
glm::mat4 ProjectionMatrix;

// Initial position : on +Z
glm::vec3 position = glm::vec3(3, 3, 0);

// Initial horizontal angle : toward -Z
float horizontalAngle = PI;
// Initial vertical angle : none
float verticalAngle = 0.0f;
// Initial Field of View
float initialFoV = 72.0f;
float extFov = 53.0792;
float speed = 1.5f; // 3 units / second
float mouseSpeed = 0.005f;
float pointSize = 8;
float kdt = 0, kd = 0, kt = 1, kc = 10, sigma2 = 1,kdist=10;

glm::mat4 getViewMatrix() {
	return ViewMatrix;
}
glm::mat4 getProjectionMatrix() {
	return ProjectionMatrix;
}
glm::vec3 getPosition() {
	return position;
}
glm::vec3 getDirection() {
	return glm::vec3(
		cos(verticalAngle) * sin(horizontalAngle),
		sin(verticalAngle),
		cos(verticalAngle) * cos(horizontalAngle)
	);
}
glm::vec2 getAngles() {
	return vec2(horizontalAngle, verticalAngle);
}
float getExtFOV() {
	return extFov;
}
int getPointSize() {
	return (int)pointSize;
}
float getkdt() {
	return kdt;
}
float getkd() {
	return kd;
}
float getkt() {
	return kt;
}
float getkc() {
	return kc;
}
float getsigma2() { // TODO: OBSOLETE
	return sigma2;
}
float getkdist() {
	return kdist;
}
void setExtFOV(float fov) {
	extFov = fov;
}
void setPointSize(int ps) {
	pointSize = ps;
}
void setkdt(float k) {
	kdt = k;
}
void setkd(float k) {
	kd = k;
}
void setkt(float k) {
	kt = k;
}
void setkc(float k) {
	kc = k;
}
void setsigma2(float k) { // TODO: OBSOLETE
	sigma2 = k;
}
void setkdist(float k) {
	kdist = k;
}





bool vDown = false;



void computeMatricesFromInputs(GLFWwindow* window) {
	// glfwGetTime is called only once, the first time this function is called
	static double lastTime = glfwGetTime();
	// Compute time difference between current and last frame
	double currentTime = glfwGetTime();
	float deltaTime = float(currentTime - lastTime);
	// Get mouse position
	double xpos, ypos;
	glfwGetCursorPos(window, &xpos, &ypos);
	// Reset mouse position for next frame
	glfwSetCursorPos(window, 1024 / 2, 768 / 2);
	// Compute new orientation
	horizontalAngle += mouseSpeed * float(1024 / 2 - xpos);
	verticalAngle += mouseSpeed * float(768 / 2 - ypos);
	if (horizontalAngle > PI)
		horizontalAngle -= 2 * PI;
	if (horizontalAngle < -PI)
		horizontalAngle += 2 * PI;
	if (verticalAngle > PI)
		verticalAngle -= 2 * PI;
	if (verticalAngle < -PI)
		verticalAngle += 2 * PI;
	// Direction : Spherical coordinates to Cartesian coordinates conversion
	glm::vec3 direction(
		cos(verticalAngle) * sin(horizontalAngle),
		sin(verticalAngle),
		cos(verticalAngle) * cos(horizontalAngle)
	);
	// Right vector
	glm::vec3 right = glm::vec3(
		sin(horizontalAngle - 3.14f / 2.0f),
		0,
		cos(horizontalAngle - 3.14f / 2.0f)
	);
	// Up vector
	glm::vec3 up = glm::cross(right, direction);
	// Move forward
	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
		position += direction * deltaTime * speed;
	}
	// Move backward
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
		position -= direction * deltaTime * speed;
	}
	// Strafe right
	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
		position += right * deltaTime * speed;
	}
	// Strafe left
	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
		position -= right * deltaTime * speed;
	}

	// Increase ext fov
	if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS) {
		extFov += deltaTime * speed * 01;
		std::cout << "Fov: " << extFov << std::endl;
	}// Increase ext fov
	if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS) {
		extFov -= deltaTime * speed * 01;
		std::cout << "Fov: " << extFov << std::endl;
	}

	//Change point size
	if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS) {
		pointSize += deltaTime * 2;
	}
	if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS) {
		pointSize -= deltaTime * 2;
	}

	if (glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS) {
		kdist += deltaTime * 5;
	}
	if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS) {
		kdist -= deltaTime * 5;
	}

	if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS) {
		kd += deltaTime * 0.1;
	}
	if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS) {
		kd -= deltaTime * 0.1;
	}

	if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS) {
		kt += deltaTime * 0.1;
	}
	if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS) {
		kt -= deltaTime * 0.1;
	}

	if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS) {
		kc += deltaTime * 100;
	}
	if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS) {
		kc -= deltaTime * 100;
	}

	float FoV = initialFoV;// - 5 * glfwGetMouseWheel(); // Now GLFW 3 requires setting up a callback for this. It's a bit too complicated for this beginner's tutorial, so it's disabled instead.
						   // Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
	ProjectionMatrix = glm::perspective(glm::radians(FoV), 1.f, 0.1f, 100.0f);
	// Camera matrix
	ViewMatrix = glm::lookAt(
		position,           // Camera is here 
		position + direction, // and looks here : at the same position, plus "direction"
		-up                  // Head is up (set to 0,-1,0 to look upside-down)
	);
	// For the next frame, the "last time" will be "now"
	lastTime = currentTime;
}