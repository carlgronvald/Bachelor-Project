#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <string>
#include "Texture.h"
class Shader
{
public:
	Shader(const char* vertexShaderFile, const char* fragmentShaderFile);
	Shader(const char* computeShaderFile);
	~Shader();

	void Bind();

	unsigned int getId();
	bool isComputeShader();
	//If it is a compute shader, compute it onto the given texture.
	void compute(int w, int h, int d, Texture texture);
	unsigned int GetUniformLocation(const char* location);
private:
	unsigned int id;
	bool computeShader;


};

