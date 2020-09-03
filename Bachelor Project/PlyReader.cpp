#include "PlyReader.h"
#include "happly.h"
#include <vector>
#include "Point.h"
#include <iostream>

happly::PLYData readPly(const char* file, int subsample) {
	std::cout << "Opening ply file... " << std::endl;
	happly::PLYData plyIn(file, true, subsample);
	std::cout << "Opened!" << std::endl;


	std::cout << "loading vertices & colors" << std::endl;
	std::vector<std::array<double, 3>> vPos = plyIn.getVertexPositions();
	std::vector<std::array<unsigned char, 3>> vCol = plyIn.getVertexColors();
	
	std::cout << "Done!" << std::endl;

	/*for (int i = 0; i < vPos.size(); i++) {
		std::cout << vPos[i][0] << "," << vPos[i][1] << "," << vPos[i][2] << std::endl;
	}*/



	std::cout << "Instantiating point cloud" << std::endl;

	PointCloud p(5);
	//p.vertexPositions = &vPos[0][0];
	//p.vertexColors = &vCol[0][0];
	//for (int i = 0; i < 1000; i++) {
	//	std::cout << p.vertexPositions[i * 3] << "," << p.vertexPositions[i * 3 + 1] << "," << p.vertexPositions[i * 3 + 2] << std::endl;
	//}

/*	PointCloud p(vPos.size());
	for (int i = 0; i < p.getLength(); i++) {
		if (i % 100000 == 0)
			std::cout << "At point " << i << std::endl;
		p.addPoint(Point(vPos[i][0], vPos[i][1], vPos[i][2], vCol[i][0], vCol[i][1], vCol[i][2]));
	}*/

	std::cout << "Returning point cloud!" << std::endl;

	return plyIn;
}