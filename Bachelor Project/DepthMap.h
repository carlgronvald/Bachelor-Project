#pragma once
#include <iostream>
#include <fstream>
#include <iterator>
#include <algorithm>
#include "Texture.h"

class DepthMap
{
public:
	DepthMap();
	DepthMap(const char* file, float minDepth, float maxDepth);
	~DepthMap();

	Texture getTexture();

	float getMinDepth();
	float getMaxDepth();
	void setMaxDepth(float depth);
	void setMinDepth(float depth);

private:
	Texture texture;
	float minDepth, maxDepth;
};

