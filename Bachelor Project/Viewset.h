#pragma once

#include "Statics.h"
#include "glm/glm.hpp"
#include "View.h"
#include "Testview.h"

class Viewset {
public:
	Viewset();
	Viewset(std::string dir);
	~Viewset();

	std::vector<View> getViews();
	View getView(int id);
	int size();
	Testview ts;

private:
	unsigned int viewCount;
	std::vector<View> views;

};
