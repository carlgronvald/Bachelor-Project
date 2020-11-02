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

#include "PointCloud.h"
#include "Point.h"
#include "PlyReader.h"
#include "controls.h"
#include "Shader.h"
#include "Buffer.h"
#include "Viewset.h"
#include "DepthMap.h"
#include "Octree.h"

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

	void renderPoints() {

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
		glm::mat4 ExternalProjectionMatrix = glm::perspective(glm::radians(getExtFOV()), 5616 / 3744.f, 0.1f, 100.0f);
		glm::mat4 ExternalMVPs[5];

		glm::vec3 ExternalViewDirs[5];
		glm::vec3 ExternalViewLocs[5];

#ifdef UPDATE_VIEWS_BASED_ON_LOCATION
		chooseViews(Position, Direction, vs);
#endif
		if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS) {
			synthesizeRelevantDepths();
			depthsSynthesized = true;
		}

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
		//ss << "(" << Position[0] << "," << Position[1] << "," << Position[2] << ") (" << Angles[0] << "," << Angles[1] << ")";
		ss << "kdt " << getkdt() << ", kd" << getkd() << ", kt" << getkt() << ", kc" << getkc();

		glfwSetWindowTitle(window, ss.str().c_str());

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

		if (depthsSynthesized)
			activeShader = &visualDepthShader;
		else
			activeShader = &visualShader;


		//Time to render to screen again
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, width, height);

		glPointSize(getPointSize());

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//Put the visual shader back
		activeShader->Bind();

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


			glActiveTexture(depthTextureSlots[i]);
			glBindTexture(GL_TEXTURE_2D, relevantDepthTextures[i].getId());
			//glBindTexture(GL_TEXTURE_2D, relevantViews[i].getDepthMap().getTexture().getId());
			minDepths[i] = relevantViews[i].getDepthMap().getMinDepth();
			maxDepths[i] = relevantViews[i].getDepthMap().getMaxDepth();

			glActiveTexture(confidenceTextureSlots[i]);
			glBindTexture(GL_TEXTURE_2D, relevantConfidenceTextures[i].getId());
		}

		int slotRefs[] = { 1,2,3,4,5 };
		glUniform1iv(activeShader->GetUniformLocation("externalTexture"), 5, &slotRefs[0]);

		if (depthsSynthesized) {
			int depthSlotRefs[] = { 6,7,8,9,10 };
			glUniform1iv(activeShader->GetUniformLocation("depthTexture"), 5, &depthSlotRefs[0]);
			glUniform1fv(activeShader->GetUniformLocation("minDepth"), 5, &minDepths[0]);
			glUniform1fv(activeShader->GetUniformLocation("maxDepth"), 5, &maxDepths[0]);
			int confidenceSlotRefs[] = { 11,12,13,14,15 };
			glUniform1iv(activeShader->GetUniformLocation("confidenceTexture"), 5, &confidenceSlotRefs[0]);

			glUniform1f(activeShader->GetUniformLocation("kdt"), getkdt());
			glUniform1f(activeShader->GetUniformLocation("kd"), getkd());
			glUniform1f(activeShader->GetUniformLocation("kt"), getkt());
			glUniform1f(activeShader->GetUniformLocation("kc"), getkc());
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


		v2Buffer.Bind();
		c2Buffer.Bind();
		n2Buffer.Bind();
		glDrawArrays(GL_POINTS, 0, c2Buffer.GetLength());
		v2Buffer.Unbind();
		c2Buffer.Unbind();
		n2Buffer.Unbind();
#else //End of NO_POINTS ifn

glBindFramebuffer(GL_FRAMEBUFFER, 0);
glViewport(0, 0, width, height);

glPointSize(getPointSize());

glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
#endif //End of NO_POINTS ifn else
	}


	// Updates relevantViews[]
	void chooseViews(glm::vec3 position, glm::vec3 direction, Viewset viewset) {
		int subsample = 1;
		std::vector<orderedView> views(viewset.size()/subsample);

		for (int i = 0; i < viewset.size()/subsample; i++) {
			views[i] = orderedView();
			views[i].viewID = i*subsample;
			float d2 = glm::length2(viewset.getView(i*subsample).getPosition() - position);
			float x = std::acos(glm::dot(direction, viewset.getView(i*subsample).getDirection()));
			views[i].weight = weight(d2, x);
			//std::cout << views[i].weight << ",";
		}
		//std::cout << std::endl;

		std::sort(views.begin(), views.end(), [](auto const &a, auto const &b) {return a.weight > b.weight;  });

		for (int i = 0; i < VIEWNUM; i++) {
			relevantViews[i] = viewset.getView(views[i].viewID);
		}
	}


	//So, I need a way to draw onto a render buffer of the same size as the texture for optimal comparisons.
	int CalculateImageDifference() {

	}

	//Generate synthetic depth maps using opengl depth textures
	void synthesizeRelevantDepths() {
		glPointSize(25);
		int vWidth = vs.getView(0).getTexture().getWidth();
		int vHeight = vs.getView(0).getTexture().getHeight();

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

		glDrawBuffer(GL_NONE);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;

		glm::mat4 projectionMatrix = glm::perspective(glm::radians(getExtFOV()), float(vWidth) / vHeight, 0.1f, 100.0f);
		glm::mat4 MVP;

		glViewport(0, 0, vWidth, vHeight); //Render on the camera

		Shader synthShader("shaders/DepthVertexShader.vertexshader", "shaders/DepthFragmentShader.fragmentshader");
		unsigned int matrixId = synthShader.GetUniformLocation("MVP");
		synthShader.Bind();

		vBuffer.Bind();
		cBuffer.Bind();
		nBuffer.Bind();
		for (int i = 0; i < VIEWNUM; i++) {
			unsigned int deletes[] = { relevantDepthTextures[i].getId(), relevantConfidenceTextures[i].getId() };
			glDeleteTextures(2, &deletes[0]);
			relevantDepthTextures[i] = Texture(vWidth, vHeight, GL_DEPTH_COMPONENT16, GL_DEPTH_COMPONENT, NULL);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, relevantDepthTextures[i].getId(), 0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			MVP = projectionMatrix * relevantViews[i].getViewMatrix() * glm::mat4(1);

			glUniformMatrix4fv(matrixId, 1, GL_FALSE, &MVP[0][0]);

			glDrawArrays(GL_POINTS, 0, vBuffer.GetLength());

			Texture tmp = convertDepthMap(relevantViews[i].getDepthMap().getTexture());
			relevantConfidenceTextures[i] = synthesizeConfidenceMap(tmp);
		}
		vBuffer.Unbind();
		cBuffer.Unbind();
		nBuffer.Unbind();

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		glDeleteFramebuffers(1, &synth_fbo);
		glDeleteRenderbuffers(1, &depthRenderbuffer);
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
		Texture confidenceTexture = Texture(depthTexture.getWidth()/4, depthTexture.getHeight()/4, GL_RGBA32F, GL_RGBA, nullptr); //Should it be RGBA32F?

		std::cout << "Generating confidence texture of width " << confidenceTexture.getWidth() << " and height " << confidenceTexture.getHeight() << std::endl;

		// A discrete laplacian kernel.
		glm::mat3 kernel{ 0,1,0,1,-4,1,0,1,0 };
		glUniformMatrix3fv(kernelShader.GetUniformLocation("convolutionMatrix"), 1, GL_FALSE, &kernel[0][0]);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, depthTexture.getId());
		glUniform1i(kernelShader.GetUniformLocation("fromTex"), 0);
		//glBindImageTexture(1, colmapDepthTexture.getId(), 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA);

		glUniform1f(kernelShader.GetUniformLocation("xratio"), 1.f / confidenceTexture.getWidth());
		glUniform1f(kernelShader.GetUniformLocation("yratio"), 1.f / confidenceTexture.getHeight());

		kernelShader.compute(confidenceTexture.getWidth(), confidenceTexture.getHeight(), 1, confidenceTexture);

		Texture tmpTexture = Texture(confidenceTexture.getWidth(), confidenceTexture.getHeight(), GL_RGBA32F, GL_RGBA, nullptr);


		//Gaussian smoothing.

		gaussShader.Bind();

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, confidenceTexture.getId());
		glUniform1i(kernelShader.GetUniformLocation("fromTex"), 0);

		glUniform1f(gaussShader.GetUniformLocation("xratio"), 1.f / confidenceTexture.getWidth());
		glUniform1f(gaussShader.GetUniformLocation("yratio"), 0);

		gaussShader.compute(confidenceTexture.getWidth(), confidenceTexture.getHeight(), 1, tmpTexture);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, tmpTexture.getId());
		glUniform1i(kernelShader.GetUniformLocation("fromTex"), 0);

		glUniform1f(gaussShader.GetUniformLocation("xratio"), 0);
		glUniform1f(gaussShader.GetUniformLocation("yratio"), 1.f / confidenceTexture.getHeight());

		gaussShader.compute(confidenceTexture.getWidth(), confidenceTexture.getHeight(), 1, confidenceTexture);

		//Smoothing, maybe it should be done with a bigger kernel or smth.

		/*kernel = glm::mat3{ 1/9., 1/9., 1/9., 1/9., 1/9., 1/9., 1/9., 1/9., 1/9. };
		glUniformMatrix3fv(kernelShader.GetUniformLocation("convolutionMatrix"), 1, GL_FALSE, &kernel[0][0]);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, confidenceTexture.getId());
		glUniform1i(kernelShader.GetUniformLocation("fromTex"), 0);

		kernelShader.compute(confidenceTexture.getWidth(), confidenceTexture.getHeight(), 1, depthTexture);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, depthTexture.getId());
		glUniform1i(kernelShader.GetUniformLocation("fromTex"), 0);

		kernelShader.compute(confidenceTexture.getWidth(), confidenceTexture.getHeight(), 1, confidenceTexture);*/

		unsigned int ctid[] = { depthTexture.getId(), tmpTexture.getId() };
		glDeleteTextures(2, &ctid[0]);

		return confidenceTexture;
	}

public:
	bool multipleViews;
	int width = 1000, height = 1000;
	unsigned int textureSlots[5] = { GL_TEXTURE1, GL_TEXTURE2, GL_TEXTURE3, GL_TEXTURE4, GL_TEXTURE5 };
	unsigned int depthTextureSlots[5] = { GL_TEXTURE6, GL_TEXTURE7, GL_TEXTURE8, GL_TEXTURE9, GL_TEXTURE10 };
	unsigned int confidenceTextureSlots[5] = { GL_TEXTURE11, GL_TEXTURE12, GL_TEXTURE13, GL_TEXTURE14, GL_TEXTURE15 };
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
	Shader convShader, kernelShader, gaussShader, visualShader, visualDepthShader;
	Shader* activeShader;
	PointCloud* pc;

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

		convShader = Shader("shaders/compute/ColmapDepthmapConverter.computeshader");
		convShader.CreateUniformLocation("fromTex");
		convShader.CreateUniformLocation("minDepth");
		convShader.CreateUniformLocation("maxDepth");

		//Create the kernel shader. That file should probably be renamed
		kernelShader = Shader("shaders/compute/DepthMapComputer.computeshader");
		kernelShader.CreateUniformLocation("fromTex");
		kernelShader.CreateUniformLocation("convolutionMatrix");
		kernelShader.CreateUniformLocation("xratio");
		kernelShader.CreateUniformLocation("yratio");

		//Create the kernel shader. That file should probably be renamed
		gaussShader = Shader("shaders/compute/OneWayGaussianBlur.computeshader");
		kernelShader.CreateUniformLocation("fromTex");
		kernelShader.CreateUniformLocation("xratio");
		kernelShader.CreateUniformLocation("yratio");


		std::cout << "making testview! " << std::endl;
#ifdef RAPID_LOAD
		Viewset vs("testview");
		happly::PLYData p = readPly("testview/outside.ply", 1);
#else
		vs = Viewset("gerrardview");

		//Time to test the depthmap conversion compute shader

		Texture resTexture = convertDepthMap(vs.getView(0).getDepthMap().getTexture());
		Texture confidenceTexture = synthesizeConfidenceMap(resTexture);

		std::cout << "Synthesized depth map confidence map" << std::endl;

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
		pc = p.pc;
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
		
		vBuffer = Buffer(sizeof(float)*3, pc->getLength(), pc->vertexPositions, 0);
		vBuffer.Bind();
		
		// And a color buffer!
		cBuffer = Buffer(sizeof(float)*3, pc->getLength(), pc->realVertexColors, 1);
		cBuffer.Bind();

		nBuffer = Buffer(sizeof(float) * 3, pc->getLength(), pc->vertexNormals, 2);
		nBuffer.Bind();
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
		}

		v2Buffer = Buffer(sizeof(float) * 3, vsize, viewLocs, 0);
		v2Buffer.Bind();

		// And a color buffer!
		v2Buffer = Buffer(sizeof(float) * 3, vsize, viewCols, 1);
		c2Buffer.Bind();

		v2Buffer = Buffer(sizeof(float) * 3, vsize, viewNs, 2);
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
		visualShader = Shader("shaders/NormalVertexShader.vertexshader", "shaders/NormalFragmentShader.fragmentshader");
#else
		visualShader = Shader("shaders/SimpleVertexShader.vertexshader", "shaders/SimpleFragmentShader.fragmentshader");
#endif
#else	
#ifdef MULTIPLE_VIEWS
		visualShader = Shader("shaders/ViewDepthlessVertexShader.vertexshader", "shaders/ViewFragmentShader.fragmentshader");
		visualDepthShader = Shader("shaders/ViewVertexShader.vertexShader", "shaders/ViewFragmentShader.fragmentshader");
#else
		Shader visualShader("shaders/TestInterpVertexShader.vertexshader", "shaders/TestInterpFragmentShader.fragmentshader");
		unsigned int ExternalTex2ID = glGetUniformLocation(visualShader.getId(), "externalTexture2");
		unsigned int ExternalMatrix2ID = glGetUniformLocation(visualShader.getId(), "viewMVP2");
		unsigned int ExternalViewDir2ID = glGetUniformLocation(visualShader.getId(), "viewDir2");
#endif
		visualDepthShader.CreateUniformLocation("depthTexture");
		visualDepthShader.CreateUniformLocation("minDepth");
		visualDepthShader.CreateUniformLocation("maxDepth");
		

		Shader** shaders = new Shader*[2]{ &visualShader, &visualDepthShader };

		for (int i = 0; i < 2; i++) {
			shaders[i]->CreateUniformLocation("externalTexture");
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
		}
		delete[] shaders;
#endif

		visualShader.Bind();

		Shader depthShader("shaders/DepthVertexShader.vertexshader", "shaders/DepthFragmentShader.fragmentshader");
		
		unsigned int depthMatrixId = glGetUniformLocation(depthShader.getId(), "MVP");


		//synthesizeDepth();
		//std::cout << "Generated synthetic depth maps!" << std::endl;


		Shader* activeShader;
		activeShader = &visualShader;

		std::cout << "Time to render" << std::endl;
		/* Loop until the user closes the window */
		while (!glfwWindowShouldClose(window))
		{

			renderPoints();

			//Drawing textures onto screen

			drawTextureOnScreen(relevantViews[0].getTexture().getId(), 0, 0, width / 4, height / 4, debugShader, dqBuffer, debugTexId);
			if (this->depthsSynthesized) {
				drawTextureOnScreen(relevantDepthTextures[0].getId(), width/4, 0, width / 4, height / 4, debugShader, dqBuffer, debugTexId);
				drawTextureOnScreen(relevantConfidenceTextures[0].getId(), width/2, 0, width / 4, height / 4, debugShader, dqBuffer, debugTexId);
			}
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
#endif
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