#pragma once
#include <GL/glew.h>
class Buffer
{
public:
	Buffer();
	Buffer(unsigned int dataSize, unsigned int dataLength, void* dataStart, unsigned int attribNumber);
	~Buffer();

	void Bind() {
		glBindBuffer(GL_ARRAY_BUFFER, id);
		glVertexAttribPointer(attribNumber, 3, GL_FLOAT, GL_FALSE, dataSize, 0);
		glEnableVertexAttribArray(attribNumber);
	}

	void Unbind() {
		glDisableVertexAttribArray(attribNumber);
	}

	unsigned int GetLength() {
		return length;
	}

private:
	unsigned int id;
	void* dataPointer;
	unsigned int usage;
	unsigned int attribNumber;
	unsigned int dataSize;
	unsigned int length;

};

