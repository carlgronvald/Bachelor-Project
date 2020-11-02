#include "DepthMap.h"

DepthMap::DepthMap()
{
}

DepthMap::DepthMap(const char* file, float minDepth, float maxDepth) : minDepth(minDepth), maxDepth(maxDepth) { //TODO: WE ALSO NEED TO KNOW WHAT DEPTH IT CORRESPONDS TO SOMEHOW
	std::cout << "Making depth texture" << std::endl;
	texture = Texture(file,true);
	std::cout << "Made!" << std::endl;
}


DepthMap::~DepthMap()
{
}

Texture DepthMap::getTexture() {
	return texture;
}
Texture DepthMap::getConfidenceTexture() {
	return confidenceTexture;
}

float DepthMap::getMinDepth() {
	return minDepth;
}
float DepthMap::getMaxDepth() {
	return maxDepth;
}
void DepthMap::setMaxDepth(float depth) {
	this->maxDepth = depth;
}
void DepthMap::setMinDepth(float depth) {
	this->minDepth = depth;
}
void DepthMap::setTexture(const Texture& texture) {
	this->texture = texture;
}


// Converts a colmap depth texture into a linear depth texture
void DepthMap::convertDepthMap(Shader conversionShader) {
	resTexture = Texture(texture.getWidth(), texture.getHeight(), GL_RGBA32F, GL_RGBA, nullptr);
	conversionShader.Bind();

	std::cout << "Converting depthmap texture "<<texture.getId() <<"(" << texture.getWidth() << "," << texture.getHeight() << ") into texture " <<resTexture.getId() << std::endl;

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture.getId());
	glUniform1i(conversionShader.GetUniformLocation("fromTex"), 0);

	conversionShader.compute(resTexture.getWidth(), resTexture.getHeight(), 1, resTexture);
}

//Creates the depth map confidence map for one view
// Use the colmap depth texture converted
void DepthMap::synthesizeConfidenceMap(Shader kernelShader, Shader conversionShader) {
	convertDepthMap(conversionShader);
	kernelShader.Bind();
	std::cout << "Restexture id: " << resTexture.getId() << std::endl;
	confidenceTexture = Texture(resTexture.getWidth(), resTexture.getHeight(), GL_RGBA32F, GL_RGBA, nullptr); //Should it be RGBA32F?

	std::cout << "Generating confidence texture of width " << resTexture.getWidth() << " and height " << resTexture.getHeight() << std::endl;

	// A discrete laplacian kernel.
	glm::mat3 kernel{ 0,1,0,1,-4,1,0,1,0 };
	glUniformMatrix3fv(kernelShader.GetUniformLocation("convolutionMatrix"), 1, GL_FALSE, &kernel[0][0]);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, resTexture.getId());
	glUniform1i(kernelShader.GetUniformLocation("fromTex"), 0);
	//glBindImageTexture(1, colmapDepthTexture.getId(), 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA);

	kernelShader.compute(confidenceTexture.getWidth(), confidenceTexture.getHeight(), 1, confidenceTexture);
	
}