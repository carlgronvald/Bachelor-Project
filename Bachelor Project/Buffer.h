#pragma once
#include <GL/glew.h>
class Buffer
{
public:
	Buffer(unsigned int dataSize, unsigned int dataLength, void* dataStart, unsigned int attribNumber);
	~Buffer();

	void Bind() {
		glVertexAttribPointer(attribNumber, 3, GL_FLOAT, GL_FALSE, dataSize, 0);
		glEnableVertexAttribArray(attribNumber);
	}

	void Unbind() {
		glDisableVertexAttribArray(attribNumber);
	}
private:
	unsigned int id;
	void* dataPointer;
	unsigned int usage;
	unsigned int attribNumber;
	unsigned int dataSize;

};

