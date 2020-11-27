//Entry point for the program
#pragma once
//#define SIMPLE
#define MULTIPLE_VIEWS					//Whether more than one view should be used
#define UPDATE_VIEWS_BASED_ON_LOCATION	//Whether views should be changed out based on location
//#define NORMALS						//Renders vertices with colors based on normals
//#define RAPID_LOAD					//Loads a fast, small dataset
//#define NO_POINTS						//Doesn't load nor render points

#define VIEWNUM 5 //Note: This is not fully implemented yet. It is the number of views used.

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdlib.h>
#include <time.h>
#include "happly.h"
#include <chrono>

#include "PointCloud.h"
#include "Point.h"
#include "PlyReader.h"
#include "controls.h"
#include "Shader.h"
#include "Buffer.h"
#include "Viewset.h"
#include "DepthMap.h"
#include "Octree.h"
#include "Testview.h"

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



	int init() {
		curPic = 0; //TODO: FIGURE OUT WHAT SCREENSHOT WE'RE AT TO AVOID OVERRIDING
#ifdef MULTIPLE_VIEWS
		multipleViews = true;
#endif
		stbi_set_flip_vertically_on_load(true);
		stbi_flip_vertically_on_write(true);

		/* Initialize the library */
		if (!glfwInit())
			return -1;

		/* Create a windowed mode window and its OpenGL context */
		window = glfwCreateWindow(width, height, "Photogrammetric Renderer", NULL, NULL);
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

		glPointSize(8);
		glClearColor(105/255.f,189/255.f,216/255.f, 1); //105/255.f, 189/255.f, 216/255.f
		glDepthRange(0.1, 100);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);

		glfwSetKeyCallback(window, key_callback);

		return 0;
	}

	void loadShaders() {

		convShader = Shader("shaders/compute/ColmapDepthmapConverter.computeshader");
		convShader.CreateUniformLocation("fromTex");
		convShader.CreateUniformLocation("minDepth");
		convShader.CreateUniformLocation("maxDepth");

		synthConvShader = Shader("shaders/compute/SyntheticDepthmapConverter.computeshader");
		convShader.CreateUniformLocation("fromTex");

		//Create the kernel shader. That file should probably be renamed
		kernelShader = Shader("shaders/compute/DepthMapComputer.computeshader");
		kernelShader.CreateUniformLocation("fromTex");
		kernelShader.CreateUniformLocation("convolutionMatrix");
		kernelShader.CreateUniformLocation("xratio");
		kernelShader.CreateUniformLocation("yratio");

		//Create the kernel shader. That file should probably be renamed
		gaussShader = Shader("shaders/compute/OneWayGaussianBlur.computeshader");
		gaussShader.CreateUniformLocation("fromTex");
		gaussShader.CreateUniformLocation("xratio");
		gaussShader.CreateUniformLocation("yratio");
		gaussShader.CreateUniformLocation("ox");
		gaussShader.CreateUniformLocation("oy");

		downsampleShader = Shader("shaders/compute/Downsampler.computeshader");
		downsampleShader.CreateUniformLocation("fromTex");
		downsampleShader.CreateUniformLocation("xratio");
		downsampleShader.CreateUniformLocation("yratio");
		downsampleShader.CreateUniformLocation("ox");
		downsampleShader.CreateUniformLocation("oy");
#ifdef SIMPLE
#ifdef NORMALS
		visualShader = Shader("shaders/NormalVertexShader.vertexshader", "shaders/NormalFragmentShader.fragmentshader");
#else
		visualShader = Shader("shaders/SimpleVertexShader.vertexshader", "shaders/SimpleFragmentShader.fragmentshader");
#endif
#else	
#ifdef MULTIPLE_VIEWS
		visualShader = Shader("shaders/ViewDepthlessVertexShader.vertexshader", "shaders/ViewFragmentShader.fragmentshader");
		visualDepthShader = Shader("shaders/NewViewVertexShader.vertexShader", "shaders/NewViewFragmentShader.fragmentshader");
		visualPointColorShader = Shader("shaders/SimpleVertexShader.vertexshader", "shaders/SimpleFragmentShader.fragmentshader");
#else
		Shader visualShader("shaders/TestInterpVertexShader.vertexshader", "shaders/TestInterpFragmentShader.fragmentshader");
		unsigned int ExternalTex2ID = glGetUniformLocation(visualShader.getId(), "externalTexture2");
		unsigned int ExternalMatrix2ID = glGetUniformLocation(visualShader.getId(), "viewMVP2");
		unsigned int ExternalViewDir2ID = glGetUniformLocation(visualShader.getId(), "viewDir2");
#endif
		visualDepthShader.CreateUniformLocation("depthTexture");
		visualDepthShader.CreateUniformLocation("confidenceTexture");
		visualDepthShader.CreateUniformLocation("minDepth");
		visualDepthShader.CreateUniformLocation("maxDepth");


		Shader** shaders = new Shader*[3]{ &visualShader, &visualDepthShader, &visualPointColorShader };

		for (int i = 0; i < 3; i++) {
			shaders[i]->CreateUniformLocation("externalTexture");
			shaders[i]->CreateUniformLocation("colmapDepth");
			shaders[i]->CreateUniformLocation("viewMVP");

			shaders[i]->CreateUniformLocation("viewDir");
			shaders[i]->CreateUniformLocation("viewLoc");
			shaders[i]->CreateUniformLocation("camDir");
			shaders[i]->CreateUniformLocation("camLoc");
			shaders[i]->CreateUniformLocation("MVP");
			//Proportionality constants
			shaders[i]->CreateUniformLocation("kdt");
			shaders[i]->CreateUniformLocation("kd");
			shaders[i]->CreateUniformLocation("kt");
			shaders[i]->CreateUniformLocation("kc");
			shaders[i]->CreateUniformLocation("kdist");

			shaders[i]->CreateUniformLocation("screenWidth");
			shaders[i]->CreateUniformLocation("screenHeight");
		}
		debugShader = Shader("shaders/DebugVertexShader.vertexshader", "shaders/DebugFragmentShader.fragmentshader");
		debugShader.CreateUniformLocation("tex");
		delete[] shaders;
#endif
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
		return std::exp((-x*x) / sigma2);
	}

	void renderPoints(glm::mat4 MVP, glm::vec3 Position, glm::vec3 Direction, int width, int height) {

#ifdef SIMPLE
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		visualShader.Bind();


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




#ifdef MULTIPLE_VIEWS
		glm::mat4 ExternalViewMatrices[5];
		glm::mat4 ExternalProjectionMatrix = glm::perspective(glm::radians(getExtFOV()), relevantViews[0].getTexture().getWidth() / ((float)relevantViews[0].getTexture().getHeight()), 0.1f, 100.0f);
		glm::mat4 ExternalMVPs[5];

		glm::vec3 ExternalViewDirs[5];
		glm::vec3 ExternalViewLocs[5];


		for (int i = 0; i < 5; i++) {
			ExternalViewMatrices[i] = relevantViews[i].getViewMatrix();
			ExternalMVPs[i] = ExternalProjectionMatrix * ExternalViewMatrices[i];

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


#ifndef NO_POINTS
		/*			Renders depth unto framebuffer texture.


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
		nBuffer.Unbind();*/




		glPointSize(getPointSize());

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//Put the visual shader back
		activeShader->Bind();

		glUniform1i(activeShader->GetUniformLocation("screenWidth"), width);
		glUniform1i(activeShader->GetUniformLocation("screenHeight"), height);

		vBuffer.Bind();
		cBuffer.Bind();
		nBuffer.Bind();

		glUniformMatrix4fv(activeShader->GetUniformLocation("MVP"), 1, GL_FALSE, &MVP[0][0]);

#ifdef MULTIPLE_VIEWS
		glUniformMatrix4fv(activeShader->GetUniformLocation("viewMVP"), 5, GL_FALSE, &ExternalMVPs[0][0][0]);
		glUniform3fv(activeShader->GetUniformLocation("viewDir"), 5, &ExternalViewDirs[0][0]);
		glUniform3fv(activeShader->GetUniformLocation("viewLoc"), 5, &ExternalViewLocs[0][0]);

		glUniform3fv(activeShader->GetUniformLocation("camDir"), 1, &Direction[0]);
		glUniform3fv(activeShader->GetUniformLocation("camLoc"), 1, &Position[0]);
		float minDepths[5];
		float maxDepths[5];
		for (int i = 0; i < 5; i++) {
			glActiveTexture(textureSlots[i]);
			glBindTexture(GL_TEXTURE_2D, relevantViews[i].getTexture().getId());

			//glBindTexture(GL_TEXTURE_2D, relevantViews[i].getDepthMap().getTexture().getId());
			minDepths[i] = relevantViews[i].getDepthMap().getMinDepth();
			maxDepths[i] = relevantViews[i].getDepthMap().getMaxDepth();

			glActiveTexture(confidenceTextureSlots[i]);
			glBindTexture(GL_TEXTURE_2D, relevantConfidenceTextures[i].getId());
			glActiveTexture(depthTextureSlots[i]);
			glBindTexture(GL_TEXTURE_2D, relevantDepthTextures[i].getId());
			glActiveTexture(GL_TEXTURE0 + 16 + i);
			glBindTexture(GL_TEXTURE_2D, relevantViews[i].getDepthMap().getTexture().getId());
		}

		int slotRefs[] = { 1,2,3,4,5 };
		glUniform1iv(activeShader->GetUniformLocation("externalTexture"), 5, &slotRefs[0]);

		if (depthsSynthesized) {
			int confidenceSlotRefs[] = { 6,7,8,9,10 };
			int depthSlotRefs[] = { 11,12,13,14,15 };
			int cdepthSlotRefs[] = { 16,17,18,19,20 };
			glUniform1iv(activeShader->GetUniformLocation("confidenceTexture"), 5, &confidenceSlotRefs[0]);
			glUniform1iv(activeShader->GetUniformLocation("depthTexture"), 5, &depthSlotRefs[0]);
			glUniform1iv(activeShader->GetUniformLocation("colmapDepth"), 5, &cdepthSlotRefs[0]);
			glUniform1fv(activeShader->GetUniformLocation("minDepth"), 5, &minDepths[0]);
			glUniform1fv(activeShader->GetUniformLocation("maxDepth"), 5, &maxDepths[0]);

			glUniform1f(activeShader->GetUniformLocation("kdt"), getkdt());
			glUniform1f(activeShader->GetUniformLocation("kd"), getkd());
			glUniform1f(activeShader->GetUniformLocation("kt"), getkt());
			glUniform1f(activeShader->GetUniformLocation("kc"), getkc());
			glUniform1f(activeShader->GetUniformLocation("kdist"), getkdist());
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
		// This was used to input the depth as seen from the camera
		//glActiveTexture(GL_TEXTURE0);
		//glBindTexture(GL_TEXTURE_2D, depthTexture);
		//glUniform1i(DepthTexID, 0);



		//I'm not sure this is how I want to do it.
		glDrawArrays(GL_POINTS, 0, pc->getLength());

		vBuffer.Unbind();
		cBuffer.Unbind();
		nBuffer.Unbind();
		//  UNBIND TO RENDER CAMERAS
		glPointSize(72);

		activeShader->Bind();

		v2Buffer.Bind();
		c2Buffer.Bind();
		n2Buffer.Bind();
		//glDrawArrays(GL_POINTS, 0, c2Buffer.GetLength());
		v2Buffer.Unbind();
		c2Buffer.Unbind();
		n2Buffer.Unbind();
#else //End of NO_POINTS ifn

glBindFramebuffer(GL_FRAMEBUFFER, 0);
glViewport(0, 0, width, height);

glPointSize(getPointSize());

glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
#endif //End of NO_POINTS ifn else
#endif //End of SIMPLE if else
	}


	// Updates relevantViews[]
	void chooseViews(glm::vec3 position, glm::vec3 direction, Viewset viewset, int excludedId = -379) {
		int subsample = 1;
		std::vector<orderedView> views(viewset.size()/subsample);

		for (int i = 0; i < viewset.size()/subsample; i++) {
			views[i] = orderedView();
			views[i].viewID = i*subsample;
			float d2 = glm::length2(viewset.getView(i*subsample).getPosition() - position);
			float x = std::acos(glm::dot(direction, viewset.getView(i*subsample).getDirection()));
			views[i].weight = weight(d2, x);
			if (i == excludedId - 1)
				views[i].weight = 0;
			//std::cout << views[i].weight << ",";
		}
		//std::cout << std::endl;

		std::sort(views.begin(), views.end(), [](auto const &a, auto const &b) {return a.weight > b.weight;  });

		bool synth = false;
		for (int i = 0; i < VIEWNUM; i++) {
			if (relevantViews[i].getId()-1 != views[i].viewID) {
				synth = true;
			}
			relevantViews[i] = viewset.getView(views[i].viewID);
		}
		if (synth) {
			depthsSynthesized = false;
		}
	}

	Texture screenTex;
	float kfov = 0; //TODO: THIS IS DUMB. WAS -0.5F

	//Tests how far off the current render is from the real image
	//So, I need a way to draw onto a render buffer of the same size as the texture for optimal comparisons.
	double CalculateImageDifference(View testview, bool writeImages) {


		int vWidth = testview.getTexture().getWidth();
		int vHeight = testview.getTexture().getHeight();

		unsigned char* data = new unsigned char[vWidth*vHeight * 3];
		glm::mat4 projectionMatrix = glm::perspective(glm::radians(getExtFOV() + kfov), float(vWidth) / vHeight, 0.1f, 100.0f);
		glm::mat4 MVP;
		MVP = projectionMatrix * testview.getViewMatrix();
		glViewport(0, 0, vWidth, vHeight); //Render on the camera
		activeShader = &visualDepthShader;
		getRender(MVP, testview.getPosition(), testview.getDirection(), vWidth, vHeight, data, testview.getId());


		unsigned char* refData = new unsigned char[vWidth*vHeight * 3];
		glBindTexture(GL_TEXTURE_2D, testview.getTexture().getId());
		glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, &refData[0]);


		unsigned char* resData = new unsigned char[vWidth*vHeight * 3];

		long long error = 0;
		int pixels = 0;
		//TODO: ALL PIXEL COLOR VALUES SHOULD BE UPSHIFTED BY 1 SO THAT WE CAN DISCERN BACKGROUND AND COLOR
		for (int i = 0; i < vWidth*vHeight * 3; i++) {
			if (data[i] != 0) {
				long long tmpError = error;
				error = error + (long long)((refData[i] - data[i])*(refData[i] - data[i]));
				pixels++;
				if (tmpError > 0 && error < 0)
					std::cout << "error went from " << tmpError << " to " << error << std::endl;
				resData[i] = abs(refData[i] - data[i]);
			}
			else {
				resData[i] = 0;
			}
		}
		if (writeImages) {
			stbi_write_png(("screenshots/err" + padnumber(curPic) + ".png").c_str(), vWidth, vHeight, 3, resData, vWidth * 3);
			stbi_write_png(("screenshots/fot" + padnumber(curPic) + ".png").c_str(), vWidth, vHeight, 3, data, vWidth * 3);
			stbi_write_png(("screenshots/ref" + padnumber(curPic++) + ".png").c_str(), vWidth, vHeight, 3, refData, vWidth * 3);
		}

		std::cout << "error " << error << ", pixels " << pixels << std::endl;

		//screenTex = ts.getTexture();

		delete[] data;
		delete[] refData;
		delete[] resData;
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		return error/((double)pixels);
	}

	void getRender(glm::mat4 MVP, glm::vec3 position, glm::vec3 direction, int vWidth, int vHeight, unsigned char* dataOut, int excludedView=-379) {

		if (screenTex.getId() != 0) {
			unsigned int del = screenTex.getId();
			glDeleteTextures(1, &del);
		}

		glViewport(0, 0, vWidth, vHeight); //Render on the camera
		glClearColor(0, 0, 0, 1);

		unsigned int synth_fbo;
		glGenFramebuffers(1, &synth_fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, synth_fbo);
		/*if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::cout << "Framebuffer creation failed!!" << std::endl;
		return;
		}*/

		// The depth buffer
		unsigned int depthRenderbuffer;
		glGenRenderbuffers(1, &depthRenderbuffer);
		glBindRenderbuffer(GL_RENDERBUFFER, depthRenderbuffer);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, vWidth, vHeight);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRenderbuffer);

		screenTex = Texture(vWidth, vHeight, GL_RGB, GL_RGB, nullptr);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, screenTex.getId(), 0);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;

//		glm::mat4 projectionMatrix = glm::perspective(glm::radians(getExtFOV() + kfov), float(vWidth) / vHeight, 0.1f, 100.0f);
//		glm::mat4 MVP;
//		MVP = projectionMatrix * view.getViewMatrix();
		chooseViews(position, direction, vs, excludedView);
		if (!depthsSynthesized) {
			synthesizeRelevantDepths();
			depthsSynthesized = true;
		}


		glViewport(0, 0, vWidth, vHeight); //Render on the camera
		glClearColor(0, 0, 0, 1);
		renderPoints(MVP, position, direction, vWidth, vHeight);
		glReadPixels(0, 0, vWidth, vHeight, GL_RGB, GL_UNSIGNED_BYTE, &dataOut[0]);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		glDeleteFramebuffers(1, &synth_fbo);
		glDeleteRenderbuffers(1, &depthRenderbuffer);
	}

	int comparison = 0;
	void PointComparison(glm::mat4 MVP, glm::vec3 position, glm::vec3 direction, int excludedView = -379) {
		int vWidth = 1000;
		int vHeight = 1000;

		float origPointSize = getPointSize();
		Shader* oldShader = activeShader;
		pc = pcOriginal;
		genBuffers();
		setPointSize(1);
		activeShader = &visualPointColorShader;
		unsigned char* data2 = new unsigned char[vWidth*vHeight * 3];
		getRender(MVP, position, direction, vWidth, vHeight, data2, excludedView);
		setPointSize(origPointSize);
		activeShader = &visualDepthShader;

		unsigned char* data1 = new unsigned char[vWidth*vHeight * 3];
		pc = pcSubsampled;
		genBuffers();
		getRender(MVP, position, direction, vWidth, vHeight, data1, excludedView);


		stbi_write_png(("screenshots/comp" + padnumber(comparison) + "a.png").c_str(), vWidth, vHeight, 3, data1, vWidth * 3);
		stbi_write_png(("screenshots/comp" + padnumber(comparison) + "b.png").c_str(), vWidth, vHeight, 3, data2, vWidth * 3);
		comparison++;
		activeShader = oldShader;
	}
	void DepthComparison(int relevantView) {

		View view = relevantViews[relevantView];


		Texture resTexture = Texture(relevantDepthTextures[relevantView].getWidth(), relevantDepthTextures[relevantView].getHeight(), GL_RGBA32F, GL_RGBA, nullptr);

		std::cout << "Converting synth depthmap texture" << std::endl;

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, relevantDepthTextures[relevantView].getId());
		glUniform1i(synthConvShader.GetUniformLocation("fromTex"), 0);

		synthConvShader.compute(resTexture.getWidth(), resTexture.getHeight(), 1, resTexture);

		unsigned char* data1 = new unsigned char[relevantDepthTextures[relevantView].getWidth()*relevantDepthTextures[relevantView].getHeight() * 3];
		glBindTexture(GL_TEXTURE_2D, resTexture.getId());
		glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, &data1[0]);


		Texture* ctex = &(relevantConfidenceTextures[relevantView]);
		unsigned char* data2 = new unsigned char[ctex->getWidth()*ctex->getHeight() * 3];
		glBindTexture(GL_TEXTURE_2D, ctex->getId());
		glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, &data2[0]);

		Texture* reftex = &(view.getTexture());
		unsigned char* data3 = new unsigned char[reftex->getWidth()*reftex->getHeight() * 3];
		glBindTexture(GL_TEXTURE_2D, reftex->getId());
		glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, &data3[0]);

		Texture* cdepthtex = &(view.getDepthMap().getTexture());
		resTexture = Texture(cdepthtex->getWidth(), cdepthtex->getHeight(), GL_RGBA32F, GL_RGBA, nullptr);
		glBindTexture(GL_TEXTURE_2D, cdepthtex->getId());
		glUniform1i(convShader.GetUniformLocation("fromTex"), 0);
		convShader.compute(resTexture.getWidth(), resTexture.getHeight(), 1, resTexture);
		unsigned char* data4 = new unsigned char[cdepthtex->getWidth()*cdepthtex->getHeight() * 3];
		glBindTexture(GL_TEXTURE_2D, resTexture.getId());
		glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, &data4[0]);


		stbi_write_png(("screenshots/comp" + padnumber(comparison) + "a.png").c_str(), relevantDepthTextures[relevantView].getWidth(), relevantDepthTextures[relevantView].getHeight(), 3, data1, relevantDepthTextures[relevantView].getWidth() * 3);
		stbi_write_png(("screenshots/comp" + padnumber(comparison) + "b.png").c_str(), ctex->getWidth(),ctex->getHeight(), 3, data2, ctex->getWidth() * 3);
		stbi_write_png(("screenshots/comp" + padnumber(comparison) + "c.png").c_str(), reftex->getWidth(),reftex->getHeight(), 3, data3, reftex->getWidth() * 3);
		stbi_write_png(("screenshots/comp" + padnumber(comparison) + "d.png").c_str(), cdepthtex->getWidth(), cdepthtex->getHeight(), 3, data4, cdepthtex->getWidth() * 3);
		comparison++;
	}

	const int TEST_VALUE_KC = 0;
	const int TEST_VALUE_KDIST = 1;
	std::string testNames[2] = { "KC", "KDIST" };
	
	int testResultFile[2] = { 0,0 };

	void saveTestResult(std::string resultString, int value) {
		std::ofstream file;
		std::stringstream filename;
		filename << "testresults/" << testNames[value] << testResultFile[value]++ << ".txt";
		file.open(filename.str());
		file << resultString;
		file.close();
	}

	void setValue(int value, float k) {
		if (value == TEST_VALUE_KC) {
			setkc(k);
		}
		else if (value == TEST_VALUE_KDIST) {
			setkdist(k);
		}
	}
	float getValue(int value) {
		if (value == TEST_VALUE_KC) {
			return getkc();
		}
		else if (value == TEST_VALUE_KDIST) {
			return getkdist();
		}
		std::cerr << std::endl << "ERROR! UNKNOWN VALUE " << value << "! ERROR!" << std::endl << std::endl;
		return -379;
	}


	double lastErr = 0;
	// err should be an array of size [2*rangeSteps+1]
	float testValue(View testview, float midpoint, float rangeSize, int rangeSteps, int value, double* err) {
		synthesizeRelevantDepths();
		std::stringstream testResult;
		float k = 0;
		float origValue = getValue(value);

		double minErr = 255*255;
		int minErrIndex = 0;
		for (int i = -rangeSteps; i <= rangeSteps; i++) {
			k = i*rangeSize / rangeSteps + midpoint;
			if (k < 0)
				continue;

			setValue(value, k);

			err[i+rangeSteps] = CalculateImageDifference(testview, false);
			if (err[i+rangeSteps] < minErr) {
				minErr = err[i + rangeSteps];
				minErrIndex = i;
			}
			testResult << k;
			testResult << " ";
			testResult << err[i + rangeSteps];
			testResult << "\n";
		}
		setValue(value, origValue);
		saveTestResult(testResult.str() , value);
		std::cout << "Best " << testNames[value] << " found at: " << (minErrIndex*rangeSize / rangeSteps + midpoint) << std::endl;
		lastErr = minErr;
		return  (minErrIndex*rangeSize / rangeSteps + midpoint);
	}

	void testValuesForView(View testview, double** sumErrors, float* startpoints, float* rangeSize, int rangeSteps) {
		auto start = std::chrono::high_resolution_clock::now();
		std::cout << "Testing for view " << testview.getId() << std::endl;
		chooseViews(testview.getPosition(), testview.getDirection(), vs, testview.getId());
		synthesizeRelevantDepths();
		std::cout << "Testing against view with id " << relevantViews[0].getId() << ", and testview has id " << testview.getId() << std::endl;

		std::stringstream resstring;

		/*double** errors = new double*[2];
		for (int i = 0; i < 21; i++) {
			errors[i] = new double[21];
			for (int j = 0; j < 21; j++) {
				errors[i][j] = 0;
			}
		}*/

		for (int i = 0; i < rangeSteps ; i++) {
			for (int j = 0; j < rangeSteps ; j++) {
				setValue(0, startpoints[0] + rangeSize[0] * i / (rangeSteps-1));
				setValue(1, startpoints[1] + rangeSize[1] * j / (rangeSteps-1));
				std::cout << "(" << getValue(0) << "," << getValue(1) << ") ";
				double err = CalculateImageDifference(testview, false);
				sumErrors[i][j] += err;
				resstring << "(" << getValue(0) << "," << getValue(1) << "): " << err << std::endl;
			}
		}

		/*for (int i = 0; i < 2; i++) {
			testValue(testview, startpoints[i]+rangeSize, rangeSize, rangeSteps, i, errors[i]);
		}
		for (int i = 0; i < 2; i++) {
			for (int j = 0; j < 21; j++) {
				sumErrors[i][j] += errors[i][j];
			}
		}*/
		std::ofstream file;
		std::stringstream filename;
		filename << "testresults/view" << testview.getId() << ".txt";
		file.open(filename.str());
		file << resstring.str();
		file.close();

		auto end = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double> elapsed = end - start;
		std::cout << "Testing took " << elapsed.count() << " seconds" << std::endl;
	}

	void testValues() {
		int rangeSteps = 5;
		double** sumErrors = new double*[rangeSteps];
		for (int i = 0; i < rangeSteps; i++) {
			sumErrors[i] = new double[rangeSteps];
			for (int j = 0; j < rangeSteps; j++) {
				sumErrors[i][j] = 0;
			}
		}

		std::stringstream results;
		results << "KC\tKDIST" << std::endl;
		float origValues[2] = { getkc(), getkdist() };
		float startpoints[2] = { 0.f, 0.f };
		float rangeSize[2] = { 400.f, 100.f};

		for (int i = 0; i < vs.size(); i++) {
			testValuesForView(vs.getView(i), sumErrors, startpoints, rangeSize, rangeSteps);
		}
		std::cout << "Bunches and bunches of errors" << std::endl << std::endl;
		for (int i = 0; i < rangeSteps; i++) {
			for (int j = 0; j < rangeSteps; j++) {
				results << "" << i*rangeSize[0]/(rangeSteps-1) << "\t" << j*rangeSize[1] / (rangeSteps - 1) << "\t" << sumErrors[i][j] << std::endl;
				std::cout << "" << i * rangeSize[0] / (rangeSteps - 1) << "\t" << j * rangeSize[1] / (rangeSteps - 1) << "\t" << sumErrors[i][j] << std::endl;
			}
		}


		std::ofstream file;
		std::stringstream filename;
		filename << "testresults/finalResults.txt";
		file.open(filename.str());
		file << results.str();
		file.close();

		setValue(0, origValues[0]);
		setValue(1, origValues[1]);
	}
	void genBuffers() {
		vBuffer = Buffer(sizeof(float) * 3, pc->getLength(), pc->vertexPositions, 0);
		vBuffer.Bind();

		// And a color buffer!
		cBuffer = Buffer(sizeof(float) * 3, pc->getLength(), pc->realVertexColors, 1);
		cBuffer.Bind();

		nBuffer = Buffer(sizeof(float) * 3, pc->getLength(), pc->vertexNormals, 2);
		nBuffer.Bind();
	}
	//Generate synthetic depth maps using opengl depth textures
	void synthesizeRelevantDepths() {

		PointCloud* tmp = pc;
		pc = pcOriginal;
		genBuffers();
		glPointSize(25);
		int vWidth = vs.getView(0).getTexture().getWidth();
		int vHeight = vs.getView(0).getTexture().getHeight();


		glm::mat4 projectionMatrix = glm::perspective(glm::radians(getExtFOV()), float(vWidth) / vHeight, 0.1f, 100.0f);
		glm::mat4 MVP;

		glViewport(0, 0, vWidth, vHeight); //Render on the camera

		Shader synthShader("shaders/DepthVertexShader.vertexshader", "shaders/DepthFragmentShader.fragmentshader");
		unsigned int matrixId = synthShader.GetUniformLocation("MVP");
		for (int i = 0; i < VIEWNUM; i++) {
			synthShader.Bind();

			vBuffer.Bind();
			cBuffer.Bind();
			nBuffer.Bind();

			//The frame buffer
			unsigned int synth_fbo;
			glGenFramebuffers(1, &synth_fbo);
			glBindFramebuffer(GL_FRAMEBUFFER, synth_fbo);

			// The depth buffer
			unsigned int depthRenderbuffer;
			glGenRenderbuffers(1, &depthRenderbuffer);
			glBindRenderbuffer(GL_RENDERBUFFER, depthRenderbuffer);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, vWidth, vHeight);
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRenderbuffer);

			glDrawBuffer(GL_NONE);

			if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
				std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;

			unsigned int deletes[] = { relevantDepthTextures[i].getId(), relevantConfidenceTextures[i].getId() };
			glDeleteTextures(2, &deletes[0]);
			relevantDepthTextures[i] = Texture(vWidth, vHeight, GL_DEPTH_COMPONENT16, GL_DEPTH_COMPONENT, NULL);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, relevantDepthTextures[i].getId(), 0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			MVP = projectionMatrix * relevantViews[i].getViewMatrix();

			std::cout << "MVP: " << MVP[0][0] << "," << MVP[0][1] << "," << MVP[0][2] << "," << MVP[0][3] << std::endl;

			glUniformMatrix4fv(matrixId, 1, GL_FALSE, &MVP[0][0]);

			glDrawArrays(GL_POINTS, 0, vBuffer.GetLength());

			Texture tmp = convertDepthMap(relevantViews[i].getDepthMap().getTexture());
			relevantConfidenceTextures[i] = synthesizeConfidenceMap(tmp);

			glDeleteFramebuffers(1, &synth_fbo);
			glDeleteRenderbuffers(1, &depthRenderbuffer);
			vBuffer.Unbind();
			cBuffer.Unbind();
			nBuffer.Unbind();
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		depthsSynthesized = true;
		pc = tmp;
		genBuffers();
	}



	// Converts a colmap depth texture into a linear depth texture
	Texture convertDepthMap(Texture colmapDepthTexture) {
		Texture resTexture = Texture(colmapDepthTexture.getWidth(), colmapDepthTexture.getHeight(), GL_RGBA32F, GL_RGBA, nullptr);

		std::cout << "Converting depthmap texture" << std::endl;

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, colmapDepthTexture.getId());
		glUniform1i(convShader.GetUniformLocation("fromTex"), 0);

		convShader.compute(resTexture.getWidth(), resTexture.getHeight(), 1, resTexture);
		return resTexture;
	}

	//Creates the depth map confidence map for one view
	// Use the colmap depth texture converted
	Texture synthesizeConfidenceMap(Texture depthTexture) {
		kernelShader.Bind();
		Texture confidenceTexture = Texture(depthTexture.getWidth(), depthTexture.getHeight(), GL_RGBA32F, GL_RGBA, nullptr); //Should it be RGBA32F?

		std::cout << "Generating confidence texture " << confidenceTexture.getId() << "of width " << confidenceTexture.getWidth() << " and height " << confidenceTexture.getHeight() << std::endl;

		// A discrete laplacian kernel.
		glm::mat3 kernel{ 0,1,0,1,-4,1,0,1,0 };
		glUniformMatrix3fv(kernelShader.GetUniformLocation("convolutionMatrix"), 1, GL_FALSE, &kernel[0][0]);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, depthTexture.getId());
		glUniform1i(kernelShader.GetUniformLocation("fromTex"), 0);
		glUniform1f(kernelShader.GetUniformLocation("xratio"), 1.f / (confidenceTexture.getWidth()-1));
		glUniform1f(kernelShader.GetUniformLocation("yratio"), 1.f / (confidenceTexture.getHeight() - 1));
		//glBindImageTexture(1, colmapDepthTexture.getId(), 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA);


		kernelShader.compute(confidenceTexture.getWidth(), confidenceTexture.getHeight(), 1, confidenceTexture);



		//Downsampling & Smoothing
		

		Texture d1tex = Texture(confidenceTexture.getWidth()/2, confidenceTexture.getHeight()/2, GL_RGBA32F, GL_RGBA, nullptr);

		downsampleShader.Bind();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, confidenceTexture.getId());
		glUniform1i(downsampleShader.GetUniformLocation("fromTex"), 0);

		glUniform1f(downsampleShader.GetUniformLocation("xratio"), 1.f / (d1tex.getWidth() - 1));
		glUniform1f(downsampleShader.GetUniformLocation("yratio"), 1.f / (d1tex.getHeight() - 1));
		glUniform1f(downsampleShader.GetUniformLocation("ox"), 1.f / (confidenceTexture.getWidth() - 1));
		glUniform1f(downsampleShader.GetUniformLocation("oy"), 1.f / (confidenceTexture.getHeight() - 1));

		downsampleShader.compute(d1tex.getWidth(), d1tex.getHeight(), 1, d1tex);


		Texture d2tex = Texture(d1tex.getWidth() / 2, d1tex.getHeight() / 2, GL_RGBA32F, GL_RGBA, nullptr);

		downsampleShader.Bind();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, confidenceTexture.getId());
		glUniform1i(downsampleShader.GetUniformLocation("fromTex"), 0);

		glUniform1f(downsampleShader.GetUniformLocation("xratio"), 1.f / (d2tex.getWidth() - 1));
		glUniform1f(downsampleShader.GetUniformLocation("yratio"), 1.f / (d2tex.getHeight() - 1));
		glUniform1f(downsampleShader.GetUniformLocation("ox"), 1.f / (d1tex.getWidth() - 1));
		glUniform1f(downsampleShader.GetUniformLocation("oy"), 1.f / (d1tex.getHeight() - 1));

		downsampleShader.compute(d2tex.getWidth(), d2tex.getHeight(), 1, d2tex);


		//Use the one called confidenceTexture again from now on.
		unsigned int del = confidenceTexture.getId();
		confidenceTexture = d2tex;
		Texture tmpTexture = Texture(confidenceTexture.getWidth(), confidenceTexture.getHeight(), GL_RGBA32F, GL_RGBA, nullptr);

		gaussShader.Bind();

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, confidenceTexture.getId());
		glUniform1i(gaussShader.GetUniformLocation("fromTex"), 0);

		glUniform1f(gaussShader.GetUniformLocation("xratio"), 1.f / (confidenceTexture.getWidth() - 1));
		glUniform1f(gaussShader.GetUniformLocation("yratio"), 1.f / (confidenceTexture.getHeight() - 1));
		glUniform1f(gaussShader.GetUniformLocation("ox"), 0);
		glUniform1f(gaussShader.GetUniformLocation("oy"), 1.f / (confidenceTexture.getHeight() - 1));

		gaussShader.compute(confidenceTexture.getWidth(), confidenceTexture.getHeight(), 1, tmpTexture);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, tmpTexture.getId());
		glUniform1i(gaussShader.GetUniformLocation("fromTex"), 0);

		glUniform1f(gaussShader.GetUniformLocation("xratio"), 1.f / (confidenceTexture.getWidth() - 1));
		glUniform1f(gaussShader.GetUniformLocation("yratio"), 1.f / (confidenceTexture.getHeight() - 1));
		glUniform1f(gaussShader.GetUniformLocation("ox"), 1.f / (confidenceTexture.getWidth() - 1));
		glUniform1f(gaussShader.GetUniformLocation("oy"), 0);

		gaussShader.compute(confidenceTexture.getWidth(), confidenceTexture.getHeight(), 1, confidenceTexture);


		//Smoothing, maybe it should be done with a bigger kernel or smth.

		/*kernel = glm::mat3{ 1/16.f, 1/8.f, 1/16.f, 1/8.f, 1/4.f, 1/8.f, 1/16.f, 1/8.f, 1/16.f };
		glUniformMatrix3fv(kernelShader.GetUniformLocation("convolutionMatrix"), 1, GL_FALSE, &kernel[0][0]);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, confidenceTexture.getId());
		glUniform1i(kernelShader.GetUniformLocation("fromTex"), 0);

		kernelShader.compute(confidenceTexture.getWidth(), confidenceTexture.getHeight(), 1, tmpTexture);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, tmpTexture.getId());
		glUniform1i(kernelShader.GetUniformLocation("fromTex"), 0);

		kernelShader.compute(confidenceTexture.getWidth(), confidenceTexture.getHeight(), 1, confidenceTexture);*/

		unsigned int ctid[] = { depthTexture.getId(), tmpTexture.getId(), del, d1tex.getId() };
		glDeleteTextures(4, &ctid[0]);

		return confidenceTexture;
	}

public:
	bool multipleViews;
	int width = 1000, height = 1000;
	unsigned int textureSlots[5] = { GL_TEXTURE1, GL_TEXTURE2, GL_TEXTURE3, GL_TEXTURE4, GL_TEXTURE5 };
	unsigned int depthTextureSlots[5] = { GL_TEXTURE11, GL_TEXTURE12, GL_TEXTURE13, GL_TEXTURE14, GL_TEXTURE15 };
	unsigned int confidenceTextureSlots[5] = { GL_TEXTURE6, GL_TEXTURE7, GL_TEXTURE8, GL_TEXTURE9, GL_TEXTURE10 }; 
	View relevantViews[VIEWNUM];
	GLFWwindow* window;
	Viewset vs;
	//Buffers for vertices, normals, and colors
	Buffer vBuffer, nBuffer, cBuffer;
	//Buffers for cameras
	Buffer v2Buffer, n2Buffer, c2Buffer;
	bool depthsSynthesized = false;
	Texture relevantDepthTextures[VIEWNUM];
	Texture relevantConfidenceTextures[VIEWNUM];
	float clearColor[3];
	Shader convShader, synthConvShader, kernelShader, gaussShader, visualShader, visualDepthShader, visualPointColorShader, debugShader, downsampleShader, maxShader;
	Shader* activeShader;
	PointCloud* pc;
	//The point cloud that isn't subsampled
	PointCloud* pcOriginal;
	//The point cloud that is subsampled
	PointCloud* pcSubsampled;
	//Level of subsampling, N_sub = N/subsample.
	int subsample = 1;

	Application() {

	}

	int main(void)
	{
		// Init some stuff...
		int status = init();

		if (status != 0)
			return status;

		int work_grp_count[3];

		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &work_grp_count[0]);
		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, &work_grp_count[1]);
		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, &work_grp_count[2]);

		std::cout << "Work group counts x: " << work_grp_count[0] << ", y: " << work_grp_count[1] << ", z: " << work_grp_count[2] << std::endl;

		int work_grp_invocations;
		glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &work_grp_invocations);
		std::cout << "Work group invocations: " << work_grp_invocations << std::endl;



		std::cout << "making testview! " << std::endl;
#ifdef RAPID_LOAD
		Viewset vs("testview");
		happly::PLYData p = readPly("testview/outside.ply", 1);
#else
		vs = Viewset("gerrardview");

		//Time to test the depthmap conversion compute shader

		std::cout << "Now reading object" << std::endl;
#ifndef NO_POINTS
		happly::PLYData p = readPly("gerrardview/object.ply", 1);
#endif
#endif
		std::cout << vs.getViews()[0].getPosition()[0] << ", " << vs.getViews()[0].getPosition()[1] << "," << vs.getViews()[0].getPosition()[2] << std::endl;
#ifdef MULTIPLE_VIEWS
		relevantViews[0] = vs.getViews()[0];
		relevantViews[1] = vs.getViews()[1];
		relevantViews[2] = vs.getViews()[2];
		relevantViews[3] = vs.getViews()[3];
		relevantViews[4] = vs.getViews()[4];
#endif

#ifndef NO_POINTS
		pcOriginal = p.pc;
		pcSubsampled = new PointCloud(p.pc, subsample);
		pc = pcSubsampled;
		//pc->createQuadVertexPositions();
		std::cout << "Points: " << p.pc->getLength() << std::endl;
		glClearColor(pc->avgColor[0], pc->avgColor[1], pc->avgColor[2], 1);
#endif
		//SURFEL STRIPPING SEEMS UNFIT TO THE PROBLEM. I'LL TRY IMPLEMENTING IT ANYWAY, MAYBE
		//Octree oc(pc->vertexPositions, pc->realVertexColors, pc->vertexNormals, pc->getLength());


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

#ifndef NO_POINTS
		// Let's make a vertex buffer!
		genBuffers();
#endif

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
			viewCols[i * 3] = 255;
			viewCols[i * 3+1] = 255;
			viewCols[i * 3+2] = 255;
		}

		v2Buffer = Buffer(sizeof(float) * 3, vsize, viewLocs, 0);
		//v2Buffer.Bind();

		// And a color buffer!
		c2Buffer = Buffer(sizeof(float) * 3, vsize, viewCols, 1);
		//c2Buffer.Bind();

		n2Buffer = Buffer(sizeof(float) * 3, vsize, viewNs, 2);
		//n2Buffer.Bind();


		float debugQuad[18] = {
			-1.0f, -1.0f, 0.0f,
			1.0f, -1.0f, 0.0f,
			-1.0f, 1.0f, 0.0f,
			-1.0f, 1.0f, 0.0f,
			1.0f, -1.0f, 0.0f,
			1.0f, 1.0f, 0.0f
		};

		Buffer dqBuffer(sizeof(float) * 3, 6, &debugQuad[0], 0);
		


		loadShaders();

		visualShader.Bind();

		Shader depthShader("shaders/DepthVertexShader.vertexshader", "shaders/DepthFragmentShader.fragmentshader");
		
		unsigned int depthMatrixId = glGetUniformLocation(depthShader.getId(), "MVP");


		//synthesizeDepth();
		//std::cout << "Generated synthetic depth maps!" << std::endl;


		activeShader = &visualShader;

//		std::cout << "The error is " << CalculateImageDifference(vs.ts);

		std::cout << "Time to render" << std::endl;
		/* Loop until the user closes the window */
		while (!glfwWindowShouldClose(window))
		{
			//Time to render to screen again
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glViewport(0, 0, width, height);
			computeMatricesFromInputs(window);
#ifdef UPDATE_VIEWS_BASED_ON_LOCATION
			chooseViews(getPosition(), getDirection(), vs);
#endif

			if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS) {
				synthesizeRelevantDepths();
				depthsSynthesized = true;
			}

			if (glfwGetKey(window, GLFW_KEY_0) == GLFW_PRESS) {
				CalculateImageDifference(relevantViews[0], true);
			} else if (glfwGetKey(window, GLFW_KEY_6) == GLFW_PRESS) {
				testValues();
			} else if (glfwGetKey(window, GLFW_KEY_7) == GLFW_PRESS) {
				glm::mat4 ProjectionMatrix = getProjectionMatrix();
				glm::mat4 ViewMatrix = getViewMatrix();
				glm::mat4 ModelMatrix = glm::mat4(1.0);
				glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
				PointComparison(MVP, getPosition(), getDirection());
			} else if (glfwGetKey(window, GLFW_KEY_8) == GLFW_PRESS && depthsSynthesized) {
				DepthComparison(0);
			}/* else if (glfwGetKey(window, GLFW_KEY_9) == GLFW_PRESS) {
				testValue(vs.getTestview(0, 15, 15, 10, TEST_VALUE_KC);
			}*/ else if (glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS) {
				loadShaders();
			}
			else {

				std::stringstream ss;
				glm::vec3 Position = getPosition();
				glm::vec2 Angles = getAngles();
				ss << "(" << Position[0] << "," << Position[1] << "," << Position[2] << ") (" << Angles[0] << "," << Angles[1] << ")";
				ss << "kdist " << getkdist() << ", kd" << getkd() << ", kt" << getkt() << ", kc" << getkc();

				glfwSetWindowTitle(window, ss.str().c_str());

				glm::mat4 ProjectionMatrix = getProjectionMatrix();
				glm::mat4 ViewMatrix = getViewMatrix();
				if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS) {
					ViewMatrix = relevantViews[0].getViewMatrix();
				} /*else if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) {
					ViewMatrix = vs.ts.getViewMatrix();
				}*/
				glm::mat4 ModelMatrix = glm::mat4(1.0);
				glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;

				if (depthsSynthesized)
					activeShader = &visualDepthShader;
				else
					activeShader = &visualShader;
				renderPoints(MVP, getPosition(), getDirection(), width, height);

			}
			//Drawing textures onto screen

			//drawTextureOnScreen(vs.getView(0).getDepthMap().getTexture().getId(), 0, 0, width / 2, height / 2, debugShader, dqBuffer, debugTexId);

	//		drawTextureOnScreen(screenTex.getId(), width/2, 0, width / 2, height / 2, debugShader, dqBuffer, debugTexId);

			if (depthsSynthesized) {
				drawTextureOnScreen(relevantConfidenceTextures[0].getId(), 0, 0, width / 4, height / 4, debugShader, dqBuffer, debugShader.GetUniformLocation("tex"));
				drawTextureOnScreen(relevantConfidenceTextures[1].getId(), width / 4, 0, width / 4, height / 4, debugShader, dqBuffer, debugShader.GetUniformLocation("tex"));
				drawTextureOnScreen(relevantConfidenceTextures[2].getId(), 2 * width / 4, 0, width / 4, height / 4, debugShader, dqBuffer, debugShader.GetUniformLocation("tex"));
				drawTextureOnScreen(relevantConfidenceTextures[3].getId(), 3 * width / 4, 0, width / 4, height / 4, debugShader, dqBuffer, debugShader.GetUniformLocation("tex"));
			}

			//drawTextureOnScreen(relevantViews[0].getTexture().getId(), 0, 0, width / 4, height / 4, debugShader, dqBuffer, debugTexId);
			//if(this->depthsSynthesized)
			//	drawTextureOnScreen(relevantConfidenceTextures[0].getId(), 0, 0, width , height, debugShader, dqBuffer, debugTexId);
			/*if (this->depthsSynthesized) {
				drawTextureOnScreen(relevantDepthTextures[0].getId(), 0, 0, width / 4, height / 4, debugShader, dqBuffer, debugTexId);
				drawTextureOnScreen(relevantDepthTextures[1].getId(), width / 4, 0, width / 4, height / 4, debugShader, dqBuffer, debugTexId);
				drawTextureOnScreen(relevantDepthTextures[2].getId(), 2*width / 4, 0, width / 4, height / 4, debugShader, dqBuffer, debugTexId);
				drawTextureOnScreen(relevantDepthTextures[3].getId(), 3*width / 4, 0, width / 4, height / 4, debugShader, dqBuffer, debugTexId);


			}*/
			//drawTextureOnScreen(vs.getView(0).getDepthMap().resTexture.getId(), width/4, 0, width / 4, height / 4, debugShader, dqBuffer, debugTexId);
			//drawTextureOnScreen(vs.getView(0).getDepthMap().getConfidenceTexture().getId(), width / 2, 0, width / 4, height / 4, debugShader, dqBuffer, debugTexId);
			
			//drawTextureOnScreen(resTexture.getId(), width / 4, height/4, width / 4, height / 4, debugShader, dqBuffer, debugTexId);
			//drawTextureOnScreen(confidenceTexture.getId(), width / 2, height/4, width / 4, height / 4, debugShader, dqBuffer, debugTexId);

			/*if (depthsSynthesized) {
				debugShader.Bind();

				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, relevantDepthTextures[0].getId());
				glUniform1i(debugTexId, 0);

				dqBuffer.Bind();

				glDrawArrays(GL_TRIANGLES, 0, 6);

				dqBuffer.Unbind();
			}*/

			/* Swap front and back buffers */
			glfwSwapBuffers(window);

			/* Poll for and process events */
			glfwPollEvents();

		}

		glfwTerminate();
		return 0;
	}

	void drawTextureOnScreen(unsigned int texId, int x, int y, int width, int height, Shader& debugShader, Buffer& dqBuffer, unsigned int debugTexId) {

		glViewport(x, y, width, height);

		debugShader.Bind();

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texId);
		glUniform1i(debugTexId, 0);

		dqBuffer.Bind();

		glDrawArrays(GL_TRIANGLES, 0, 6);

		dqBuffer.Unbind();
	}

};

int main(void) {
	
	Application a;
	return a.main();
}