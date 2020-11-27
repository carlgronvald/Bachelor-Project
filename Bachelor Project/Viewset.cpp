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
	//testviews = std::vector<Testview>(0);


	
	for (int i = 0; i < viewCount; i++) {
		//It's gonna make both a testview and a view each iteration. Pretty cool
		//testviews[i] = Testview((int)imageCsv[0].second[i+viewCount], glm::quat(imageCsv[1].second[i + viewCount], imageCsv[2].second[i + viewCount], imageCsv[3].second[i + viewCount], imageCsv[4].second[i + viewCount]), glm::vec3(imageCsv[5].second[i + viewCount], imageCsv[6].second[i + viewCount], imageCsv[7].second[i + viewCount]), dir + "/view" + padnumber((int)imageCsv[8].second[i + viewCount]) + ".jpg");
		views[i] = View((int)imageCsv[0].second[i], glm::quat(imageCsv[1].second[i], imageCsv[2].second[i], imageCsv[3].second[i], imageCsv[4].second[i]), glm::vec3(imageCsv[5].second[i], imageCsv[6].second[i], imageCsv[7].second[i]), dir + "/view" + padnumber((int)imageCsv[8].second[i]) + ".jpg",
			dir + "/depth_treated/viewd" + padnumber((int)imageCsv[0].second[i]) + ".png", depthCsv[1].second[i], depthCsv[2].second[i]);
		
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

Testview Viewset::getTestview(int id) {
	return testviews[id];
}