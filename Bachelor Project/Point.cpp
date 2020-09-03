#include "Point.h"
#include <iostream>

Point::Point() {

	}

Point::Point(int x, int y, int z, int r, int g, int b) :
		x(x), y(y), z(z), r(r), g(g), b(b) {};

int Point::getX() {
	return x;
}
int Point::getY() {
	return y;
}
int Point::getZ() {
	return z;
}
int Point::getR() {
	return r;
}
int Point::getG() {
	return g;
}
int Point::getB() {
	return b;
}

void Point::print() {
	std::cout << "(x,y,z):(" << x << "," << y << "," << z << ") - (r,g,b):(" << r << "," << g << "," << b << ")" << std::endl;
}
