//Entry point for the program

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdlib.h>
#include <time.h>
#include "happly.h"

#include "PointCloud.h"
#include "Point.h"
#include "PlyReader.h"
#include "controls.h"
#include "Shader.h"
#include "Buffer.h"

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
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);
	}
public:

	int width = 640, height = 480;
	int main(void)
	{
		happly::PLYData p = readPly("summer_house.ply", 1);
		PointCloud* pc = p.pc;
		std::cout << "Points: " << p.pc->getLength();

		GLFWwindow* window;

		/* Initialize the library */
		if (!glfwInit())
			return -1;


		/* Create a windowed mode window and its OpenGL context */
		window = glfwCreateWindow(width,height , "Photogrammetric Renderer", NULL, NULL);
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
		
		// The depth buffer
		unsigned int depthRenderbuffer;
		glGenRenderbuffers(1, &depthRenderbuffer);
		glBindRenderbuffer(GL_RENDERBUFFER, depthRenderbuffer);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRenderbuffer);

		// We'll make a depth texture for two-pass rendering.
		unsigned int depthTexture;
		glGenTextures(1, &depthTexture);
		glBindTexture(GL_TEXTURE_2D, depthTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		
		// Let's make a vertex buffer!
		Buffer vBuffer(sizeof(float)*3, pc->getLength(), pc->vertexPositions, 0);
		vBuffer.Bind();

		// And a color buffer!
		Buffer cBuffer(sizeof(float)*3, pc->getLength(), pc->realVertexColors, 1);
		cBuffer.Bind();

		Buffer nBuffer(sizeof(float) * 3, pc->getLength(), pc->vertexNormals, 2);
		nBuffer.Bind();

		Shader shader("shaders/SimpleVertexShader.vertexshader", "shaders/SimpleFragmentShader.fragmentshader");
		unsigned int MatrixID = glGetUniformLocation(shader.getId(), "MVP");

		shader.Bind();

		std::cout << "Time to render" << std::endl;
		/* Loop until the user closes the window */
		while (!glfwWindowShouldClose(window))
		{
			/* Render here */
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			computeMatricesFromInputs(window);
			glm::mat4 ProjectionMatrix = getProjectionMatrix();
			glm::mat4 ViewMatrix = getViewMatrix();
			glm::mat4 ModelMatrix = glm::mat4(1.0);
			glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;

			glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);

			glDrawArrays(GL_POINTS, 0, pc->getLength());



			/* Swap front and back buffers */
			glfwSwapBuffers(window);

			/* Poll for and process events */
			glfwPollEvents();
		}

		glfwTerminate();
		return 0;
	}
};

int main(void) {
	Application a;
	return a.main();
}