#pragma once
#include "glm/glm.hpp"
#include "PNG.h"
#include "glm/gtc/matrix_transform.hpp"
#include "Texture.h"

class View {
public:
	View(int id, glm::vec3 position, glm::vec3 rotation, std::string imgfile);
	View();

	Texture getTexture();
	glm::vec3 getPosition();
	glm::vec3 getRotation();
	glm::mat4 getViewMatrix();

private:
	int id;
	glm::vec3 position;
	glm::vec3 rotation;
	Texture texture;
};