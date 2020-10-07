#pragma once
#include "Viewset.h"
#include "Statics.h"


Viewset::Viewset() {

}

Viewset::Viewset(std::string dir) {
	std::vector < std::pair<std::string, std::vector<float>>> imageCsv = readCsv(dir + "/images.txt", '\t');
	std::vector < std::pair<std::string, std::vector<float>>> depthCsv = readCsv(dir + "/depth_treated/depths.txt", '\t');
	viewCount = imageCsv[0].second.size();
	views = std::vector<View>(viewCount);


	
	for (int i = 0; i < viewCount; i++) {
		std::cout << "Making view " << (i + 1) << " with csv length " << imageCsv.size() << std::endl;

		views[i] = View(i, glm::quat(imageCsv[1].second[i], imageCsv[2].second[i], imageCsv[3].second[i], imageCsv[4].second[i]), glm::vec3(imageCsv[5].second[i], imageCsv[6].second[i], imageCsv[7].second[i]), dir+"/view"+padnumber((int)imageCsv[8].second[i])+".jpg",
			dir+"/depth_treated/viewd"+padnumber(i+1) + ".png", depthCsv[1].second[i], depthCsv[2].second[i]);
		
		//views[i] = View(i, glm::vec3(csv[1].second[i], csv[2].second[i], csv[3].second[i]), glm::vec3(csv[4].second[i], csv[5].second[i], csv[6].second[i]), dir+"/view"+padnumber(i+1)+".png");
	}
}

Viewset::~Viewset() {
}

std::vector<View> Viewset::getViews() {
	return views;
}

int Viewset::size() {
	return viewCount;
}

View Viewset::getView(int id) {
	return views[id];
}