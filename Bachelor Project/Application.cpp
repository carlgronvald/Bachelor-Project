//Entry point for the program

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdlib.h>
#include <time.h>
#include <iostream>
#include "happly.h"

#include "PointCloud.h"
#include "Point.h"
#include "PlyReader.h"






void init() {
	glPointSize(15);
	glClearColor(0, 0, 0, 1);
}
int main(void)
{
	happly::PLYData p = readPly("summer_house.ply");

	srand(time(NULL));
//	PointCloud p(1500000);
//
//	for (int i = 0; i < p.getLength(); i++) {
//		p.addPoint(Point(rand() % 100, rand() % 100, rand() % 100, rand() % 256, rand() % 256, rand() % 256));
		
		//std::cout << i << ": ";
		//p.getPoint(i).print();
//	}

//	PointCloud subsample(1500000 / 100);
//	for (int i = 0; i < subsample.getLength(); i++) {
//		subsample.addPoint(p.getPoint(i*(p.getLength() / subsample.getLength())));
//	}

	GLFWwindow* window;

	/* Initialize the library */
	if (!glfwInit())
		return -1;


	/* Create a windowed mode window and its OpenGL context */
	window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		return -1;
	}

	/* Make the window's context current */
	glfwMakeContextCurrent(window);


	/* Now we can init GLEW, since we have a valid context */
	if (glewInit() != GLEW_OK) {
		std::cout << "Error" << std::endl;
	}

	// Init some stuff...
	init();

//	float positions[6] = {
//		-0.5f, -0.5f,
//		0.0f, 0.5f,
//		0.5f, -0.5f
//	};
	
	// Let's make a vertex buffer!
	unsigned int buffer;
	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(float), p.pc->vertexPositions, GL_STATIC_DRAW);

	//std::vector<std::array<double, 3>> vPos = p.getVertexPositions();
	//std::vector<std::array<unsigned char, 3>> vCol = p.getVertexColors();

	/* Loop until the user closes the window */
	while (!glfwWindowShouldClose(window))
	{
		/* Render here */
		glClear(GL_COLOR_BUFFER_BIT);

		glBegin(GL_POINTS);

		for (int i = 0; i < 100000; i++) {
			glColor3f(p.pc->vertexColors[i*3+0] / 255.f, p.pc->vertexColors[i*3+1] / 255.f, p.pc->vertexColors[i*3+2] / 255.f);
			glVertex2f(p.pc->vertexPositions[i*3+0], p.pc->vertexPositions[i*3+1]);
		}

		glEnd();

		/* Swap front and back buffers */
		glfwSwapBuffers(window);

		/* Poll for and process events */
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}