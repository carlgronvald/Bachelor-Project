#include "Shader.h"
#include "Statics.h"

unsigned int CompileShader(const std::string& source, unsigned int type) {
	unsigned int id = glCreateShader(type);
	const char* src = source.c_str();
	glShaderSource(id, 1, &src, nullptr);
	glCompileShader(id);

	GLint compiled;
	glGetShaderiv(id, GL_COMPILE_STATUS, &compiled);
	if (compiled != GL_TRUE)
	{
		GLsizei log_length = 0;
		GLchar message[1024];
		glGetShaderInfoLog(id, 1024, &log_length, message);

		std::cerr << message << std::endl;
	}


	return id;
}
unsigned int CreateShader(const std::string& vertexShader, const std::string &fragmentShader) {
	unsigned int program = glCreateProgram();
	unsigned int vs = CompileShader(vertexShader, GL_VERTEX_SHADER);
	unsigned int fs = CompileShader(fragmentShader, GL_FRAGMENT_SHADER);

	glAttachShader(program, vs);
	glAttachShader(program, fs);
	glLinkProgram(program);
	glValidateProgram(program);

	glDeleteShader(vs);
	glDeleteShader(fs);

	std::cout << "Compiled shaders! " << std::endl;

	return program;
}

Shader::Shader(const char* vertexShaderFile, const char* fragmentShaderFile)
{
	this->id = CreateShader(readFile(vertexShaderFile), readFile(fragmentShaderFile));
}


Shader::~Shader()
{
}
