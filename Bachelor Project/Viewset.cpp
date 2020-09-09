#pragma once
#include "Viewset.h"
#include "Statics.h"


Viewset::Viewset(std::string dir) {
	std::vector < std::pair<std::string, std::vector<float>>> csv = readCsv(dir + "/views.txt");
	viewCount = csv[0].second.size();
	views = std::vector<View>(viewCount);
	
	for (int i = 0; i < viewCount; i++) {
		views[i] = View(i, glm::vec3(csv[1].second[i], csv[2].second[i], csv[3].second[i]), glm::vec3(csv[4].second[i], csv[5].second[i], csv[6].second[i]), dir+"/view"+padnumber(i+1)+".png");
	}
}

Viewset::~Viewset() {
}

std::vector<View> Viewset::getViews() {
	return views;
}

