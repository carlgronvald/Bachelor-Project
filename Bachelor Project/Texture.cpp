#include "Texture.h"


Texture::Texture() {

}

Texture::Texture(std::string file)
{
	p = PNG(file.c_str());
	

	glGenTextures(1, &id);
	glBindTexture(GL_TEXTURE_2D, id);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, p.getWidth(), p.getHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, p.dataReference());
	glBindTexture(GL_TEXTURE_2D, 0);

	width = p.getWidth();
	height = p.getHeight();
}


Texture::~Texture()
{
}

unsigned int Texture::getId() {
	return id;
}
