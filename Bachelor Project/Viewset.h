#pragma once

#include "Statics.h"
#include "glm/glm.hpp"
#include "PNG.h"
#include "View.h"

class Viewset {
public:
	Viewset(std::string dir);
	~Viewset();

	std::vector<View> getViews();
private:
	unsigned int viewCount;
	std::vector<View> views;

};
