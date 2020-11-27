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
	Testview getTestview(int id);
	int size();

private:
	unsigned int viewCount;
	std::vector<View> views;
	std::vector<Testview> testviews;

};
