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


	return program;
}

unsigned int CreateShader(const std::string& computeShader) {
	unsigned int program = glCreateProgram();
	unsigned int cs = CompileShader(computeShader, GL_COMPUTE_SHADER);

	glAttachShader(program, cs);
	glLinkProgram(program);

	glValidateProgram(program);
	glDeleteShader(cs);

	return program;
}

Shader::Shader() {

}

Shader::Shader(const char* vertexShaderFile, const char* fragmentShaderFile)
{
	this->id = CreateShader(readFile(vertexShaderFile), readFile(fragmentShaderFile));
	std::cout << "Compiled shaders " << vertexShaderFile << " and " << fragmentShaderFile << "!" << std::endl;
	computeShader = false;
	uniforms = std::map<std::string, unsigned int>();
}

Shader::Shader(const char* computeShaderFile) {
	
	this -> id = CreateShader(readFile(computeShaderFile));
	std::cout << "Compiled compute shader " << computeShaderFile << "!" << std::endl;
	computeShader = true;
	uniforms = std::map<std::string, unsigned int>();
}


Shader::~Shader()
{
}

bool Shader::isComputeShader() {
	return computeShader;
}

unsigned int Shader::getId() {
	return id;
}

void Shader::Bind() {
	glUseProgram(id);
}

void Shader::compute(int w, int h, int d, Texture texture) {
	if (!computeShader)
		return;
	glBindImageTexture(0, texture.getId(), 0, GL_FALSE, 0, GL_WRITE_ONLY, texture.getFormat());
	Bind();
	glDispatchCompute(w, h, d);
}

unsigned int Shader::GetUniformLocation(std::string location) {
	return uniforms[location];
}

void Shader::CreateUniformLocation(std::string location) {
	uniforms.emplace(location, glGetUniformLocation(id, location.c_str()));
}