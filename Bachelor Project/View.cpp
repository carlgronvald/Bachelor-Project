#include "View.h"

View::View() {

}
View::View(int id, glm::vec3 position, glm::vec3 rotation, std::string imgfile) : id(id), position(position), rotation(rotation), texture(imgfile) {
	
}
View::View(int id, glm::quat quaternion, glm::vec3 translation, std::string imgfile) : id(id), position(translation), texture(imgfile) {
	rotation = glm::eulerAngles(quaternion);

}

Texture View::getTexture() {
	return texture;
}
glm::vec3 View::getPosition() {
	return position;
}
glm::vec3 View::getRotation() {
	return rotation;
}

glm::mat4 View::getViewMatrix() {
	// Direction : Spherical coordinates to Cartesian coordinates conversion
	glm::vec3 direction(
		cos(rotation[1]) * sin(rotation[0]),
		sin(rotation[1]),
		cos(rotation[1]) * cos(rotation[0])
	);
	glm::vec3 up(0, 1, 0);

	// Camera matrix
	glm::mat4 ViewMatrix = glm::lookAt(
		position,           // Camera is here 
		position + direction, // and looks here : at the same position, plus "direction"
		up                  // Head is up (set to 0,-1,0 to look upside-down)
	);

	return ViewMatrix;
}