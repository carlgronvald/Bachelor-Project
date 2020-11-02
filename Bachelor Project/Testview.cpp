#include "Testview.h"



Testview::Testview()
{
}

Testview::Testview(int id, glm::quat quaternion, glm::vec3 translation, std::string imgfile) :View(id, quaternion, translation, imgfile, "NAN", 0, 0) {
	
}

Testview::~Testview()
{
}
