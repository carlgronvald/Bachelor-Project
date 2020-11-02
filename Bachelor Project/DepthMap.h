#pragma once
#include <iostream>
#include <fstream>
#include <iterator>
#include <algorithm>
#include "Texture.h"
#include "Shader.h"
#include "glm/glm.hpp"

class DepthMap
{
public:
	DepthMap();
	DepthMap(const char* file, float minDepth, float maxDepth);
	~DepthMap();

	Texture getTexture();
	Texture getConfidenceTexture();

	float getMinDepth();
	float getMaxDepth();
	void setMaxDepth(float depth);
	void setMinDepth(float depth);
	void setTexture(const Texture& texture);
	void synthesizeConfidenceMap(Shader kernelShader, Shader conversionShader);

	//TODO: MUST BE REMOVED OR MADE PRIVATE
	Texture resTexture;
private:
	Texture texture;
	Texture confidenceTexture;
	float minDepth, maxDepth;
	void convertDepthMap(Shader conversionShader);
};

