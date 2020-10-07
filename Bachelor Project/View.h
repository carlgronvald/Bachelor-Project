#pragma once
#include "glm/glm.hpp"
#include "glm/gtx/quaternion.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "Texture.h"
#include "DepthMap.h"

class View {
public:
	View(int id, glm::quat quaternion, glm::vec3 translation, std::string imgfile, std::string depthfile, float minDepth, float maxDepth);
	View();

	Texture getTexture();
	glm::vec3 getPosition();
	glm::vec3 getDirection();
	glm::mat3 getRotation();
	glm::mat4 getViewMatrix();

	DepthMap getDepthMap();

private:
	int id;
	glm::vec3 position;
	glm::mat3 rotation;
	Texture texture;
	DepthMap depthMap;
};