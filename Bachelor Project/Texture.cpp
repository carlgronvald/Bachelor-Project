#include "Texture.h"


Texture::Texture() {

}

Texture::Texture(std::string file, bool rgba)
{
	std::cout << "Starting reading texture from file " << file << std::endl;
	int bpp;
	unsigned char* rgb_image = stbi_load(file.c_str(), &width, &height, &bpp, rgba ? 4 : 3);
	glGenTextures(1, &id);
	glBindTexture(GL_TEXTURE_2D, id);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
	glTexImage2D(GL_TEXTURE_2D, 0, rgba ? GL_RGBA : GL_RGB, width,height, 0, rgba ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, rgb_image);
	glBindTexture(GL_TEXTURE_2D, 0);
	this->format = rgba ? GL_RGBA : GL_RGB;
	this->type = rgba ? GL_RGBA : GL_RGB;
	delete[] rgb_image;
}

//Type doesn't do anything rn TODO
Texture::Texture(int width, int height, int format, int type, unsigned char* data) : width(width), height(height) {
	glGenTextures(1, &id);
	glBindTexture(GL_TEXTURE_2D, id);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
	glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, type, GL_UNSIGNED_BYTE, data);

	this->format = format;
	this->type = type;
	glBindTexture(GL_TEXTURE_2D, 0);
}

Texture::~Texture() 
{
}

unsigned int Texture::getId() {
	return id;
}
int Texture::getType() {
	return type;
}
int Texture::getFormat() {
	return format;
}

int Texture::getWidth() {
	return width;
}

int Texture::getHeight() {
	return height;
}