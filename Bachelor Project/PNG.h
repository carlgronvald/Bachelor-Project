#pragma once
#include <vector>
#include "lodepng.h"
#include <iostream>

class PNG
{
public:
	PNG();
	PNG(const char* filename);
	~PNG();

	int getWidth() {
		return width;
	}

	int getHeight() {
		return height;
	}

	unsigned char* dataReference() {
		return &image[0];
	}

private:
	std::vector<unsigned char> image;
	unsigned int width, height;
};

