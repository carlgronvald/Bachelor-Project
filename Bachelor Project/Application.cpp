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

//srand(time(NULL));

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



class Application {

	void init() {
		glPointSize(8);
		glClearColor(0, 0, 0, 1);
	}
	int main(void)
	{
		happly::PLYData p = readPly("summer_house.ply");


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
		
		// Let's make a vertex buffer!
		unsigned int buffer;
		glGenBuffers(1, &buffer);
		glBindBuffer(GL_ARRAY_BUFFER, buffer);
		glBufferData(GL_ARRAY_BUFFER, p.pc->getLength() * sizeof(float), p.pc->vertexPositions, GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
		glEnableVertexAttribArray(0);


		/* Loop until the user closes the window */
		while (!glfwWindowShouldClose(window))
		{
			/* Render here */
			glClear(GL_COLOR_BUFFER_BIT);

			glDrawArrays(GL_POINTS, 0, p.pc->getLength());


			/* Swap front and back buffers */
			glfwSwapBuffers(window);

			/* Poll for and process events */
			glfwPollEvents();
		}

		glfwTerminate();
		return 0;
	}
}