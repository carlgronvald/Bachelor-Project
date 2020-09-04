#pragma once
#include <string>
#include <iostream>
#include <fstream>
#include <streambuf>

std::string readFile(const char* filename) {
	std::string text;
	std::ifstream myfile(filename);

	myfile.seekg(0, std::ios::end);
	text.reserve(myfile.tellg());
	myfile.seekg(0, std::ios::beg);

	text.assign((std::istreambuf_iterator<char>(myfile)),
		std::istreambuf_iterator<char>());
	return text;
}