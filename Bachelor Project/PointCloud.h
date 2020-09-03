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
		realVertexColors = new float[1];
	}
	~PointCloud() {
		delete[] vertexPositions;
		delete[] vertexColors;
		delete[] vertexNormals;
		delete[] realVertexColors;
	}

	int getLength() {
		return length;
	}

	//
	// Creates the realVertexColors array
	//
	void createRealVertexColors() {
		realVertexColors = new float[3 * length];
		for (int i = 0; i < length * 3; i++) {
			realVertexColors[i] = vertexColors[i] / 255.f;
		}
	}

	float* vertexPositions;
	unsigned char* vertexColors;
	float* realVertexColors;
	float* vertexNormals;
	const int length;
};