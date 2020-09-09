#pragma once

#include "Statics.h"
#include "glm/glm.hpp"
#include "PNG.h"

class Viewset {
public:
	Viewset(std::string dir);
	~Viewset();
private:
	int viewCount;
	View* views;

};

class View {
public:
	View(int id, glm::vec3 position, glm::vec3 rotation, std::string imgfile);

	PNG getImage();
	glm::vec3 getPosition();
	glm::vec3 getRotation();

private:
	View();
	int id;
	glm::vec3 position;
	glm::vec3 rotation;
	PNG image;

	friend class Viewset;
};