#include "DepthMap.h"

DepthMap::DepthMap()
{
}

DepthMap::DepthMap(const char* file, float minDepth, float maxDepth) : minDepth(minDepth), maxDepth(maxDepth) { //TODO: WE ALSO NEED TO KNOW WHAT DEPTH IT CORRESPONDS TO SOMEHOW
	std::cout << "Making depth texture" << std::endl;
	texture = Texture(file,true);
	std::cout << "Made!" << std::endl;
}


DepthMap::~DepthMap()
{
}

Texture DepthMap::getTexture() {
	return texture;
}

float DepthMap::getMinDepth() {
	return minDepth;
}
float DepthMap::getMaxDepth() {
	return maxDepth;
}
void DepthMap::setMaxDepth(float depth) {
	this->maxDepth = depth;
}
void DepthMap::setMinDepth(float depth) {
	this->minDepth = depth;
}
void DepthMap::setTexture(const Texture& texture) {
	this->texture = texture;
}