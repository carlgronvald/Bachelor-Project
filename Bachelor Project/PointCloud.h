#pragma once
#include "glm/glm.hpp"
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

	float* quadVertexPositions;
	void createQuadVertexPositions() {
		quadVertexPositions = new float[3 * 6 * length];


		glm::vec3 ax1(1, 0, 0);
		glm::vec3 ax2(0, 1, 0);

		glm::vec3 v1, v2; //Orthogonal vector 1&2.
		glm::vec3 p; //Position of vertex

		glm::vec3 p1, p2, p3, p4;

		for (int i = 0; i < length; i++) {
			glm::vec3 normal(vertexNormals[i * 3], vertexNormals[i * 3 + 1], vertexNormals[i * 3 + 2]);
			if (glm::dot(normal, ax1) > glm::dot(normal, ax2)) {
				v1 = glm::normalize(glm::cross(normal, ax2));
			}
			else {
				v1 = glm::normalize(glm::cross(normal, ax1));
			}

			v2 = glm::normalize(glm::cross(v1, normal));
			
			v1 = v1 * 0.05f;
			v2 = v2 * 0.05f;

			p = glm::vec3(vertexPositions[i * 3], vertexPositions[i * 3 + 1], vertexPositions[i * 3 + 2]);

			p1 = p - v1 - v2;
			p2 = p - v1 + v2;
			p3 = p + v1 + v2;
			p4 = p + v1 - v2;

			quadVertexPositions[i * 18] = p1[0];
			quadVertexPositions[i * 18+1] = p1[1];
			quadVertexPositions[i * 18+2] = p1[2];

			quadVertexPositions[i * 18 + 3] = p2[0];
			quadVertexPositions[i * 18 + 4] = p2[1];
			quadVertexPositions[i * 18 + 5] = p2[2];

			quadVertexPositions[i * 18 + 6] = p4[0];
			quadVertexPositions[i * 18 + 7] = p4[1];
			quadVertexPositions[i * 18 + 8] = p4[2];

			quadVertexPositions[i * 18 + 9] = p2[0];
			quadVertexPositions[i * 18 + 10] = p2[1];
			quadVertexPositions[i * 18 + 11] = p2[2];

			quadVertexPositions[i * 18 + 12] = p4[0];
			quadVertexPositions[i * 18 + 13] = p4[1];
			quadVertexPositions[i * 18 + 14] = p4[2];

			quadVertexPositions[i * 18 + 15] = p3[0];
			quadVertexPositions[i * 18 + 16] = p3[1];
			quadVertexPositions[i * 18 + 17] = p3[2];
		}
	}
};