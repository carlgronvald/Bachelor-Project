#include "DepthMap.h"

DepthMap::DepthMap()
{
}

DepthMap::DepthMap(const char* file) { //TODO: WE ALSO NEED TO KNOW WHAT DEPTH IT CORRESPONDS TO SOMEHOW
	texture = Texture(file,true);
}


DepthMap::~DepthMap()
{
}

Texture DepthMap::getTexture() {
	return texture;
}