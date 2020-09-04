#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <string>
class Shader
{
public:
	Shader(const char* vertexShaderFile, const char* fragmentShaderFile);
	~Shader();

	void Bind() {
		glUseProgram(id);
	}

	unsigned int getId() {
		return id;
	}

private:
	unsigned int id;


};

