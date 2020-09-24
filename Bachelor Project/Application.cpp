//Entry point for the program
#pragma once
//#define SIMPLE
#define MULTIPLE_VIEWS					//Whether more than one view should be used
#define UPDATE_VIEWS_BASED_ON_LOCATION	//Whether views should be changed out based on location
//#define NORMALS						//Renders vertices with colors based on normals
//#define RAPID_LOAD					//Loads a fast, small dataset

#define VIEWNUM 5 //Note: This is not fully implemented yet. It is the number of views used.

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
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"



static int curPic;
struct orderedView {
	int viewID;
	float weight;
};


void savePicture() {
	unsigned char* pixels;
	int screenStats[4];

	//Get width/height of window
	glGetIntegerv(GL_VIEWPORT, screenStats);

	//Generate pixel array
	pixels = new unsigned char[1000*1000 * 3];
	glReadPixels(0, 0, 1000, 1000, GL_RGB, GL_UNSIGNED_BYTE, pixels);
	stbi_write_png(("screenshots/shot" + padnumber(curPic++) + ".png").c_str(), 1000, 1000, 3, pixels, 1000 * 3);
	std::cout << "Wrote file " << "screenshots/shot" << padnumber(curPic - 1) << ".png" << std::endl;
	delete[] pixels;
}
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (key == GLFW_KEY_F && action == GLFW_PRESS) {
		savePicture();
	}
}
class Application {

	void init() {
		glPointSize(8);
		glClearColor(1,1,1, 1); //105/255.f, 189/255.f, 216/255.f
		glDepthRange(0.1, 100);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);

		glfwSetKeyCallback(window, key_callback);
	}


#define VIEW_CHOICE_METHOD_BEST_DOT // This one works by just choosing the view with the best dot product - entirely view dependant

	View ChooseView(glm::vec3 position, glm::vec3 direction, Viewset viewset) {
		int closestView = 0;
		double bestDot = -1;



#ifdef VIEW_CHOICE_METHOD_BEST_DOT
		for (int i = 0; i < viewset.getViews().size(); i++) {
			if (glm::dot(viewset.getViews()[i].getDirection(), direction) > bestDot) {
				bestDot = glm::dot(viewset.getViews()[i].getDirection(), direction);
				closestView = i;
			}
		}
#endif

		return viewset.getViews()[closestView];
	}

	const float sigma2 = 2;
	float weight(float d2, float x) {
		return std::exp((-d2 * x*x -d2) / sigma2);
	}


	// Updates relevantViews[]
	void chooseViews(glm::vec3 position, glm::vec3 direction, Viewset viewset) {
		std::vector<orderedView> views(viewset.size());

		for (int i = 0; i < viewset.size(); i++) {
			views[i] = orderedView();
			views[i].viewID = i;
			float d2 = glm::length2(viewset.getView(i).getPosition() - position);
			float x = std::acos(glm::dot(direction, viewset.getView(i).getDirection()));
			views[i].weight = weight(d2, x);
			//std::cout << views[i].weight << ",";
		}
		//std::cout << std::endl;

		std::sort(views.begin(), views.end(), [](auto const &a, auto const &b) {return a.weight > b.weight;  });

		for (int i = 0; i < VIEWNUM; i++) {
			relevantViews[i] = viewset.getView(views[i].viewID);
		}
	}

public:
	bool multipleViews;
	int width = 1000, height = 1000;
	unsigned int textureSlots[5] = { GL_TEXTURE1, GL_TEXTURE2, GL_TEXTURE3, GL_TEXTURE4, GL_TEXTURE5 };

	View relevantViews[VIEWNUM];
	GLFWwindow* window;


	int main(void)
	{
		curPic = 0; //TODO: FIGURE OUT WHAT PICUTRE WE'RE AT TO AVOID OVERRIDING
#ifdef MULTIPLE_VIEWS
		multipleViews = true;
#endif
		stbi_set_flip_vertically_on_load(true);
		stbi_flip_vertically_on_write(true);

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
#ifdef RAPID_LOAD
		Viewset vs("testview");
		happly::PLYData p = readPly("testview/outside.ply", 1);
#else
		Viewset vs("gerrardview");
		happly::PLYData p = readPly("gerrardview/object.ply", 1);
#endif
		std::cout << vs.getViews()[0].getPosition()[0] << ", " << vs.getViews()[0].getPosition()[1] << "," << vs.getViews()[0].getPosition()[2] << std::endl;
#ifdef MULTIPLE_VIEWS
		relevantViews[0] = vs.getViews()[0];
		relevantViews[1] = vs.getViews()[1];
		relevantViews[2] = vs.getViews()[2];
		relevantViews[3] = vs.getViews()[3];
		relevantViews[4] = vs.getViews()[4];
#endif

		PointCloud* pc = p.pc;
		//pc->createQuadVertexPositions();
		std::cout << "Points: " << p.pc->getLength() << std::endl;
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


		// TEMPORARY FOR TESTING
		int vsize = vs.getViews().size();
		float* viewLocs = new float[vs.getViews().size() * 3];
		float* viewCols = new float[vs.getViews().size() * 3];
		float* viewNs = new float[vs.getViews().size() * 3];

		for (int i = 0; i < vs.getViews().size(); i++) {
			View v = vs.getViews()[i];
			viewLocs[i * 3] = v.getPosition()[0];
			viewLocs[i * 3 + 1] = v.getPosition()[1];
			viewLocs[i * 3 + 2] = v.getPosition()[2];
		}

		Buffer v2Buffer(sizeof(float) * 3, vsize, viewLocs, 0);
		v2Buffer.Bind();

		// And a color buffer!
		Buffer c2Buffer(sizeof(float) * 3, vsize, viewCols, 1);
		c2Buffer.Bind();

		Buffer n2Buffer(sizeof(float) * 3, vsize, viewNs, 2);
		n2Buffer.Bind();


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
#ifdef NORMALS
		Shader visualShader("shaders/NormalVertexShader.vertexshader", "shaders/NormalFragmentShader.fragmentshader");
#else
		Shader visualShader("shaders/SimpleVertexShader.vertexshader", "shaders/SimpleFragmentShader.fragmentshader");
#endif
#else	
#ifdef MULTIPLE_VIEWS
		Shader visualShader("shaders/ViewVertexShader.vertexshader", "shaders/ViewFragmentShader.fragmentshader");
#else
		Shader visualShader("shaders/TestInterpVertexShader.vertexshader", "shaders/TestInterpFragmentShader.fragmentshader");
		unsigned int ExternalTex2ID = glGetUniformLocation(visualShader.getId(), "externalTexture2");
		unsigned int ExternalMatrix2ID = glGetUniformLocation(visualShader.getId(), "viewMVP2");
		unsigned int ExternalViewDir2ID = glGetUniformLocation(visualShader.getId(), "viewDir2");
#endif
		unsigned int DepthTexID = glGetUniformLocation(visualShader.getId(), "depthTexture");
		unsigned int ExternalTexID = glGetUniformLocation(visualShader.getId(), "externalTexture");
		unsigned int ExternalMatrixID = glGetUniformLocation(visualShader.getId(), "viewMVP");

		unsigned int ExternalViewDirID = glGetUniformLocation(visualShader.getId(), "viewDir");
		unsigned int ExternalViewLocID = glGetUniformLocation(visualShader.getId(), "viewLoc");
		unsigned int camDirID = glGetUniformLocation(visualShader.getId(), "camDir");
		unsigned int camLocID = glGetUniformLocation(visualShader.getId(), "camLoc");
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

			vBuffer.Bind();
			cBuffer.Bind();
			nBuffer.Bind();

			glDrawArrays(GL_POINTS, 0, pc->getLength());

			vBuffer.Unbind();
			cBuffer.Unbind();
			nBuffer.Unbind();

			/* Swap front and back buffers */
			glfwSwapBuffers(window);

			/* Poll for and process events */
			glfwPollEvents();
#else
			computeMatricesFromInputs(window);


			glm::vec3 Position = getPosition();
			glm::vec2 Angles = getAngles();
			glm::vec3 Direction = getDirection();

			View relevantView = ChooseView(Position, Direction, vs);

			glm::mat4 ProjectionMatrix = getProjectionMatrix();
			glm::mat4 ViewMatrix = getViewMatrix();
			if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS) {
				ViewMatrix = relevantView.getViewMatrix();
			}
			glm::mat4 ModelMatrix = glm::mat4(1.0);
			glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;

#ifdef MULTIPLE_VIEWS
			glm::mat4 ExternalViewMatrices[5];
			glm::mat4 ExternalProjectionMatrix = glm::perspective(glm::radians(getExtFOV()), 5616/3744.f, 0.1f, 100.0f);
			glm::mat4 ExternalMVPs[5];

			glm::vec3 ExternalViewDirs[5];
			glm::vec3 ExternalViewLocs[5];

#ifdef UPDATE_VIEWS_BASED_ON_LOCATION
			chooseViews(Position, Direction, vs);
#endif


			for (int i = 0; i < 5; i++) {
				ExternalViewMatrices[i] = relevantViews[i].getViewMatrix();
				ExternalMVPs[i] = ExternalProjectionMatrix * ExternalViewMatrices[i] * ModelMatrix;
				
				ExternalViewDirs[i] = relevantViews[i].getDirection();
				ExternalViewLocs[i] = relevantViews[i].getPosition();
			}
#else
			glm::mat4 ExternalViewMatrix = relevantView.getViewMatrix();
			glm::mat4 ExternalProjectionMatrix = glm::perspective(glm::radians(67.f), 3 / 4.f, 0.1f, 100.0f);
			glm::mat4 ExternalMVP = ExternalProjectionMatrix * ExternalViewMatrix * ModelMatrix;

			glm::mat4 ExternalViewMatrix2 = vs.getViews()[1].getViewMatrix();
			glm::mat4 ExternalProjectionMatrix2 = glm::perspective(glm::radians(67.f), 3 / 4.f, 0.1f, 100.0f);
			glm::mat4 ExternalMVP2 = ExternalProjectionMatrix2 * ExternalViewMatrix2 * ModelMatrix;
#endif



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

			glPointSize(getPointSize());

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			//Put the visual shader back
			visualShader.Bind();

			vBuffer.Bind();
			cBuffer.Bind();
			nBuffer.Bind();

			glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);

#ifdef MULTIPLE_VIEWS
			glUniformMatrix4fv(ExternalMatrixID, 5, GL_FALSE, &ExternalMVPs[0][0][0]);
			glUniform3fv(ExternalViewDirID, 5, &ExternalViewDirs[0][0]);
			glUniform3fv(ExternalViewLocID, 5, &ExternalViewLocs[0][0]);

			glUniform3fv(camDirID, 1, &Direction[0]);
			glUniform3fv(camLocID, 1, &Position[0]);
			for (int i = 0; i < 5; i++) {
				glActiveTexture(textureSlots[i]);
				glBindTexture(GL_TEXTURE_2D, relevantViews[i].getTexture().getId());
				int slotRefs[] = { 1,2,3,4,5 };
				glUniform1iv(ExternalTexID, 5, &slotRefs[0]);
			}
#else
			glUniformMatrix4fv(ExternalMatrixID, 1, GL_FALSE, &ExternalMVP[0][0]);
			glUniformMatrix4fv(ExternalMatrix2ID, 1, GL_FALSE, &ExternalMVP2[0][0]);
			glm::vec3 vector(1, 0, 0);
			glm::vec3 vector2(0.0, 1, 1);
			glUniform3fv(ExternalViewDirID, 1, &vector[0]);
			glUniform3fv(ExternalViewDir2ID, 1, &vector2[0]);
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, relevantView.getTexture().getId());
			glUniform1i(ExternalTexID, 1);
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, vs.getViews()[1].getTexture().getId());
			glUniform1i(ExternalTex2ID, 2);

#endif
			//Put in an active texture or smth 
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, depthTexture);
			glUniform1i(DepthTexID, 0);

			

			//I'm not sure this is how I want to do it.
			glDrawArrays(GL_POINTS, 0, pc->getLength());

			vBuffer.Unbind();
			cBuffer.Unbind();
			nBuffer.Unbind();


			//  UNBIND TO RENDER CAMERAS
			glPointSize(72);


			v2Buffer.Bind();
			c2Buffer.Bind();
			n2Buffer.Bind();
			glDrawArrays(GL_POINTS, 0, vsize);
			v2Buffer.Unbind();
			c2Buffer.Unbind();
			n2Buffer.Unbind();

			glViewport(width/2, 0, width / 2, height / 2);
			debugShader.Bind();

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, vs.getViews()[0].getTexture().getId());
			glUniform1i(debugTexId, 0);

			dqBuffer.Bind();

			//glDrawArrays(GL_TRIANGLES, 0, 6);

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