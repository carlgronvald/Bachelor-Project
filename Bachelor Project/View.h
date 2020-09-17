#pragma once
#include "glm/glm.hpp"
#include "glm/gtx/quaternion.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "Texture.h"

class View {
public:
	View(int id, glm::quat quaternion, glm::vec3 translation, std::string imgfile);
	View();

	Texture getTexture();
	glm::vec3 getPosition();
	glm::mat3 getRotation();
	glm::mat4 getViewMatrix();

private:
	int id;
	glm::vec3 position;
	glm::mat3 rotation;
	Texture texture;
};