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
	DepthMap(const char* file);
	~DepthMap();

	Texture getTexture();

private:
	Texture texture;
};

