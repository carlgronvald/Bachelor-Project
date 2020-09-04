#include "Buffer.h"



Buffer::Buffer(unsigned int dataSize, unsigned int dataLength, void* dataStart, unsigned int attribNumber) : attribNumber(attribNumber), dataSize(dataSize)
{
	glGenBuffers(1, &id);
	glBindBuffer(GL_ARRAY_BUFFER, id);
	glBufferData(GL_ARRAY_BUFFER, dataSize*dataLength, dataStart, GL_STATIC_DRAW);

}


Buffer::~Buffer()
{
}
