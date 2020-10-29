#pragma once
#include <math.h>
#include <limits>
#include <iostream>
#include <string>
struct Surfel
{
public:
	float x, y, z;
	float r, g, b;
	float nx, ny, nz;
};

class Node
{
public:
	Node(float minX, float maxX, float minY, float maxY, float minZ, float maxZ) :minX(minX), maxX(maxX), minY(minY), maxY(maxY), minZ(minZ), maxZ(maxZ) {
		for (int i = 0; i < 8; i++)
			children[i] = nullptr;
		leaves = nullptr;
	}
	~Node() {
		for (int i = 0; i < 8; i++)
			if(children[i] != nullptr)
				delete children[i];
		if(leaves != nullptr)
			delete[] leaves;
	}
	//Tells us whether the node should be subdivided
	bool ToDivide() {
		if (n <= 1)
			return false;
		if (n > 40)
			return true;
		bool divide = false;
		for (int i = 0; i < n; i++) {
			//if (leaves[i].nx * nx + leaves[i].ny * ny + leaves[i].nz * nz z 0.15) //Angle requirement - This one will fail because the normals are of bad quality :(
			//	divide = true;
			//if (abs((leaves[i].x - x) * nx + (leaves[i].y - y)*ny + (leaves[i].z - z)*nz) / maxDist > 0.15 ) //Geometric displacement requirement
			//	divide = true;

		}
		return divide;
	}

	void FillNodes(float* vpos, float* vcol, float* vnorm, int n) {
		leaves = new Surfel[n];
		for (int i = 0; i < n*3; i+=3) {
			leaves[i / 3].x = vpos[i];
			leaves[i / 3].y = vpos[i+1];
			leaves[i / 3].z = vpos[i+2];
			leaves[i / 3].r = vcol[i];
			leaves[i / 3].g = vcol[i + 1];
			leaves[i / 3].b = vcol[i + 2];
			leaves[i / 3].nx = vnorm[i];
			leaves[i / 3].ny = vnorm[i+1];
			leaves[i / 3].nz = vnorm[i+2];
		}
		this->n = n;
	}

	void CalculateValues() {
		x = 0; y = 0; z = 0; nx = 0; ny = 0; nz = 0;
		for (int i = 0; i < n; i++) {
			x += leaves[i].x;
			y += leaves[i].y;
			z += leaves[i].z;
			nx += leaves[i].nx;
			ny += leaves[i].ny;
			nz += leaves[i].nz;
		}
		x /= n; y /= n; z /= n; nx /= n; ny /= n; nz /= n;
	}

	void Divide() {
		std::cout << "Dividing node (" << minX << "," << minY << "," << minZ << "), (" << maxX << "," << maxY << "," << maxZ << ")" << std::endl;
		

		int sizes[8] = { 0,0,0,0,0,0,0,0 };
		int* leafDistribution = new int[n];

		for (int i = 0; i < 8; i++) {
			children[i] = new Node(minX + Width() / 2 * (i % 2), minX + Width() / 2 * (i % 2 + 1),
				minY + Height() / 2 * ((i / 2) % 2), minY + Height() / 2 * ((i / 2) % 2 + 1),
				minZ + Depth() / 2 * ((i / 4) % 2), minZ + Depth() / 2 * ((i / 4) % 2 + 1));
			std::cout << "Child " << i << " has extreme points (" << children[i]->minX << "," << children[i]->minY << "," << children[i]->minZ << "), ("
				<< children[i]->maxX << "," << children[i]->maxY << "," << children[i]->maxZ << ")" << std::endl;
		}

		Surfel* s;
		for (int i = 0; i < n; i++) {
			s = &leaves[i];
			leafDistribution[i] = (s->x < x ? 0 : 1) + (s->y < y ? 0 : 2) + (s->z < z ? 0 : 4);
			sizes[leafDistribution[i]]++;
		}

		for (int i = 0; i < 8; i++) {
			if(sizes[i] > 0)
				children[i]->leaves = new Surfel[sizes[i]];
			std::cout << sizes[i] << " leaves in child " << i << std::endl;
		}

		int ld = -1;
		for (int i = 0; i < n; i++) {
			ld = leafDistribution[i];
			children[ld]->leaves[children[ld]->n] = Surfel(leaves[i]);
			children[ld]->n++;
			//std::cout << "Point (" << leaves[i].x << "," << leaves[i].y << "," << leaves[i].z << ") in child " << ld << std::endl;
		}

		for (int i = 0; i < 8; i++) {
			children[i]->CalculateValues();
			if (children[i]->ToDivide()) {
				std::cout << "Dividing node " << i << std::endl;
				std::string bob;
				std::getline(std::cin, bob);
				children[i]->Divide();
			}
		}

		delete[] leafDistribution;
		//delete[] leaves;
		//leaves = nullptr;
		n = 0;
	}

private:
	float minX, maxX, minY, maxY, minZ, maxZ;
	Node * children[8];
	Surfel * leaves;
	int n;
	//Average normal vector of all the surfels.
	float nx, ny, nz;
	//Barycenter of Node
	float x, y, z;
	float maxDist;

	float Width() {
		return maxX - minX;
	}
	float Height() {
		return maxY - minY;
	}
	float Depth() {
		return maxZ - minZ;
	}

};


class Octree
{
public:
	Octree(float* vpos, float* vcol, float* vnorm, int n) : parent(0,0,0,0,0,0) {
		float minX = std::numeric_limits<float>::max();
		float maxX = -std::numeric_limits<float>::max();
		float minY = minX, minZ = minX;
		float maxY = maxX, maxZ = maxX;
		for (int i = 0; i < n*3; i+=3) {
			//std::cout << "Looking at point (" << vpos[i] << ", " << vpos[i+1] << "," << vpos[i+2] << ")" << std::endl;
			if (vpos[i] < minX)
				minX = vpos[i];
			if (vpos[i] > maxX)
				maxX = vpos[i];
			if (vpos[i + 1] < minY)
				minY = vpos[i + 1];
			if (vpos[i + 1] > maxY)
				maxY = vpos[i+1];
			if (vpos[i + 2] < minZ)
				minZ = vpos[i + 2];
			if (vpos[i + 2] > maxZ)
				maxZ = vpos[i + 2];
		}
		std::string bob;
		std::getline(std::cin, bob);
		std::cout << "Creating parent node (" << minX << "," << minY << "," << minZ << "), (" << maxX << "," << maxY << "," << maxZ << ")" << std::endl;
		parent = Node(minX, maxX, minY, maxY, minZ, maxZ);
		parent.FillNodes(vpos, vcol, vnorm, n);
		parent.CalculateValues();
		parent.Divide();
	}
	~Octree();

private:
	Node parent;
};