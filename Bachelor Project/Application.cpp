//Entry point for the program
#pragma once
//#define SIMPLE

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
#include "Viewset.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
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
		glPointSize(24);
		glClearColor(105/255.f, 189/255.f, 216/255.f, 1);
		glDepthRange(0.01, 100);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);
	}
public:

	int width = 640, height = 480;
	int main(void)
	{
		stbi_set_flip_vertically_on_load(true);

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

		glewExperimental = true;
		/* Now we can init GLEW, since we have a valid context */
		if (glewInit() != GLEW_OK) {
			std::cout << "Error" << std::endl;
		}

		// Init some stuff...
		init();
		std::cout << "making testview! " << std::endl;
		Viewset vs("testview");
		std::cout << vs.getViews()[0].getPosition()[0] << ", " << vs.getViews()[0].getPosition()[1] << "," << vs.getViews()[0].getPosition()[2] << std::endl;

		happly::PLYData p = readPly("testview/outside.ply", 1);
		PointCloud* pc = p.pc;
		std::cout << "Points: " << p.pc->getLength();
#ifndef SIMPLE
		unsigned int framebufferName = 0;
		glGenFramebuffers(1, &framebufferName);
		glBindFramebuffer(GL_FRAMEBUFFER, framebufferName);

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
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);

		glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthTexture, 0);

		glDrawBuffer(GL_NONE);

		if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			return -1;
#endif

		
		// Let's make a vertex buffer!
		Buffer vBuffer(sizeof(float)*3, pc->getLength(), pc->vertexPositions, 0);
		vBuffer.Bind();
		
		// And a color buffer!
		Buffer cBuffer(sizeof(float)*3, pc->getLength(), pc->realVertexColors, 1);
		cBuffer.Bind();

		Buffer nBuffer(sizeof(float) * 3, pc->getLength(), pc->vertexNormals, 2);
		nBuffer.Bind();


		float debugQuad[18] = {
			-1.0f, -1.0f, 0.0f,
			1.0f, -1.0f, 0.0f,
			-1.0f, 1.0f, 0.0f,
			-1.0f, 1.0f, 0.0f,
			1.0f, -1.0f, 0.0f,
			1.0f, 1.0f, 0.0f
		};

		Buffer dqBuffer(sizeof(float) * 3, 6, &debugQuad[0], 0);
		
		Shader debugShader("shaders/DebugVertexShader.vertexshader", "shaders/DebugFragmentShader.fragmentshader");
		unsigned int debugTexId = glGetUniformLocation(debugShader.getId(), "tex");


#ifdef SIMPLE
		Shader visualShader("shaders/SimpleVertexShader.vertexshader", "shaders/SimpleFragmentShader.fragmentshader");
#else	
		Shader visualShader("shaders/InterpVertexShader.vertexshader", "shaders/InterpFragmentShader.fragmentshader");
		unsigned int DepthTexID = glGetUniformLocation(visualShader.getId(), "depthTexture");
		unsigned int ExternalTexID = glGetUniformLocation(visualShader.getId(), "externalTexture");
		unsigned int ExternalMatrixID = glGetUniformLocation(visualShader.getId(), "viewMVP");
#endif
		unsigned int MatrixID = glGetUniformLocation(visualShader.getId(), "MVP");

		visualShader.Bind();

		Shader depthShader("shaders/DepthVertexShader.vertexshader", "shaders/DepthFragmentShader.fragmentshader");
		
		unsigned int depthMatrixId = glGetUniformLocation(depthShader.getId(), "MVP");


		std::cout << "Time to render" << std::endl;
		/* Loop until the user closes the window */
		while (!glfwWindowShouldClose(window))
		{
#ifdef SIMPLE
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			visualShader.Bind();

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
#else
			computeMatricesFromInputs(window);
			glm::mat4 ProjectionMatrix = getProjectionMatrix();
			glm::mat4 ViewMatrix = getViewMatrix();
			if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS) {
				ViewMatrix = vs.getViews()[0].getViewMatrix();
			}
			glm::mat4 ExternalViewMatrix = vs.getViews()[0].getViewMatrix();
			glm::mat4 ModelMatrix = glm::mat4(1.0);
			glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
			glm::mat4 ExternalMVP = ProjectionMatrix * ExternalViewMatrix * ModelMatrix;



			glm::vec3 Position = getPosition();
			glm::vec2 Angles = getAngles();

			std::stringstream ss;
			ss << "(" << Position[0] << "," << Position[1] << "," << Position[2] << ") (" << Angles[0] << "," << Angles[1] << ")";
			glfwSetWindowTitle(window, ss.str().c_str());

			glBindFramebuffer(GL_FRAMEBUFFER, depthRenderbuffer);
			glViewport(0, 0, width, height); //Render on the entire framebuffer.

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			depthShader.Bind();

			vBuffer.Bind();
			cBuffer.Bind();
			nBuffer.Bind();

			glUniformMatrix4fv(depthMatrixId, 1, GL_FALSE, &MVP[0][0]);

			glDrawArrays(GL_POINTS, 0, pc->getLength());

			vBuffer.Unbind();
			cBuffer.Unbind();
			nBuffer.Unbind();


			//Time to render to screen again
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glViewport(0, 0, width, height);
			
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			//Put the visual shader back
			visualShader.Bind();

			vBuffer.Bind();
			cBuffer.Bind();
			nBuffer.Bind();

			glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
			glUniformMatrix4fv(ExternalMatrixID, 1, GL_FALSE, &ExternalMVP[0][0]);

			//Put in an active texture or smth 
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, depthTexture);
			glUniform1i(DepthTexID, 0);

			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, vs.getViews()[0].getTexture().getId());
			glUniform1i(ExternalTexID, 1);
			

			//I'm not sure this is how I want to do it.
			glDrawArrays(GL_POINTS, 0, pc->getLength());

			vBuffer.Unbind();
			cBuffer.Unbind();
			nBuffer.Unbind();

			glViewport(width/2, 0, width / 2, height / 2);
			debugShader.Bind();

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, vs.getViews()[0].getTexture().getId());
			glUniform1i(debugTexId, 0);

			dqBuffer.Bind();

			glDrawArrays(GL_TRIANGLES, 0, 6);

			dqBuffer.Unbind();

			/* Swap front and back buffers */
			glfwSwapBuffers(window);

			/* Poll for and process events */
			glfwPollEvents();
#endif
		}

		glfwTerminate();
		return 0;
	}
};

int main(void) {
	Application a;
	return a.main();
}