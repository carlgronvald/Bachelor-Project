#pragma once
class Point
{
public:
	Point();

	Point(int x, int y, int z, int r, int g, int b);
	int getX();
	int getY();
	int getZ();
	int getR();
	int getG();
	int getB();
	void print();
private:
	int x, y, z;
	int r, g, b;
};

