#include "View.h"

View::View() {

}
View::View(int id, glm::quat quaternion, glm::vec3 translation, std::string imgfile) : id(id), position(translation), texture(imgfile) {
	rotation = glm::toMat3(quaternion);
}

Texture View::getTexture() {
	return texture;
}
glm::vec3 View::getPosition() {
	return -glm::transpose(rotation)*position;
}
glm::mat3 View::getRotation() {
	return rotation;
}

glm::mat4 View::getViewMatrix() {
	// Direction : Spherical coordinates to Cartesian coordinates conversion

	glm::mat3 nrt = -glm::transpose(rotation);

	glm::vec3 direction = nrt * glm::vec3(0, 0, -1);
	glm::vec3 up(0, 1, 0);
	glm::vec3 nPosition = nrt * position;


	// Camera matrix
	glm::mat4 ViewMatrix = glm::lookAt(
		nPosition,           // Camera is here 
		nPosition + direction, // and looks here : at the same position, plus "direction"
		nrt*up                  // Head is up (set to 0,-1,0 to look upside-down)
	);

	return ViewMatrix;
}