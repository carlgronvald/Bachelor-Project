#include "PNG.h"



PNG::PNG()
{
}

PNG::PNG(const char* filename) {
	//decode
	unsigned error = lodepng::decode(image, width, height, filename);

	//if there's an error, display it
	if (error) std::cout << "decoder error " << error << ": " << lodepng_error_text(error) << std::endl;
}

PNG::~PNG()
{
}