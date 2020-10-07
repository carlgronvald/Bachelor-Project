#include "View.h"

View::View() {

}
View::View(int id, glm::quat quaternion, glm::vec3 translation, std::string imgfile, std::string depthfile, float minDepth, float maxDepth) : id(id), position(translation), texture(imgfile) {
	rotation = glm::toMat3(quaternion);
	depthMap = DepthMap(depthfile.c_str(), minDepth, maxDepth);
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
glm::vec3 View::getDirection() {
	return -glm::transpose(rotation) * glm::vec3(0, 0, -1);
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

DepthMap View::getDepthMap() {
	return depthMap;
}