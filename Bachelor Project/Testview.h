#pragma once
#include "View.h"
class Testview :
	public View
{
public:
	Testview();
	Testview(int id, glm::quat quaternion, glm::vec3 translation, std::string imgfile);
	~Testview();
};

