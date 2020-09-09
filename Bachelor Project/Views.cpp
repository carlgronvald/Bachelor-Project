#pragma once
#include "Views.h"
#include "Statics.h"


Viewset::Viewset(std::string dir) {
	std::vector < std::pair<std::string, std::vector<float>>> csv = readCsv(dir + "/views.txt");
	this->viewCount = csv[0].second.size;
	views = new View[viewCount];
	
	for (int i = 0; i < viewCount; i++) {
		views[i] = View(i, glm::vec3(csv[1].second[i], csv[2].second[i], csv[3].second[i]), glm::vec3(csv[4].second[i], csv[5].second[i], csv[6].second[i]), dir+"/view"+padnumber(i+1)+".png");
	}
}

Viewset::~Viewset() {
	delete[] views;
}


View::View() {

}

View::View(int id, glm::vec3 position, glm::vec3 rotation, std::string imgfile) : id(id), position(position), rotation(rotation) {
	image = PNG(imgfile.c_str());
}

PNG View::getImage() {
	return image;
}
glm::vec3 View::getPosition() {
	return position;
}
glm::vec3 View::getRotation() {
	return rotation;
}