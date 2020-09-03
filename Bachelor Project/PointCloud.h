#pragma once
class PointCloud {
public:
	PointCloud()
		: length(0) {
	}
	PointCloud(const int length) 
		: length(length) {
		vertexPositions = new float[length*3];
		vertexColors = new unsigned char[length * 3];
		vertexNormals = new float[length * 3];
	}

	int getLength() {
		return length;
	}

	float* vertexPositions;
	unsigned char* vertexColors;
	float* vertexNormals;
	const int length;
};