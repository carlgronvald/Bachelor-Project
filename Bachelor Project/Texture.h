#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <string>
#include<iostream>

#include "stb_image.h"

class Texture
{
public:
	//Reads a PNG from file and makes a texture out of it.
	Texture();
	Texture(std::string file, bool rgba=false);
	Texture(int width, int height, int type, unsigned char* data);
	~Texture();

	unsigned int getId();
private:
	unsigned int id;
	int width, height;
};

