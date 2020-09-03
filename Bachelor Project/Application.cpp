//Entry point for the program

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdlib.h>
#include <time.h>
#include <iostream>
#include <fstream>
#include <streambuf>
#include "happly.h"

#include "PointCloud.h"
#include "Point.h"
#include "PlyReader.h"
#include "controls.h"

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

	static unsigned int CompileShader(const std::string& source, unsigned int type) {
		unsigned int id = glCreateShader(type);
		const char* src = source.c_str();
		glShaderSource(id, 1, &src, nullptr);
		glCompileShader(id);

		GLint compiled;
		glGetShaderiv(id, GL_COMPILE_STATUS, &compiled);
		if (compiled != GL_TRUE)
		{
			GLsizei log_length = 0;
			GLchar message[1024];
			glGetShaderInfoLog(id, 1024, &log_length, message);

			std::cout << message << std::endl;
		}


		return id;
	}

	static unsigned int CreateShader(const std::string& vertexShader, const std::string &fragmentShader) {
		unsigned int program = glCreateProgram();
		unsigned int vs = CompileShader(vertexShader, GL_VERTEX_SHADER);
		unsigned int fs = CompileShader(fragmentShader, GL_FRAGMENT_SHADER);

		glAttachShader(program, vs);
		glAttachShader(program, fs);
		glLinkProgram(program);
		glValidateProgram(program);

		glDeleteShader(vs);
		glDeleteShader(fs);

		std::cout << "Compiled shaders! " << std::endl;

		return program;
	}

	static std::string readFile(const char* filename) {
		std::string text;
		std::ifstream myfile(filename);

		myfile.seekg(0, std::ios::end);
		text.reserve(myfile.tellg());
		myfile.seekg(0, std::ios::beg);

		text.assign((std::istreambuf_iterator<char>(myfile)),
			std::istreambuf_iterator<char>());
		return text;
	}

	void init() {
		glPointSize(8);
		glClearColor(0, 0, 0, 1);
	}
public:
	int main(void)
	{
		happly::PLYData p = readPly("summer_house.ply", 5);


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
		unsigned int vertexBuffer;
		glGenBuffers(1, &vertexBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
		glBufferData(GL_ARRAY_BUFFER, p.pc->getLength() * sizeof(float), p.pc->vertexPositions, GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
		glEnableVertexAttribArray(0);
		// And a color buffer!
		unsigned int colorBuffer;
		glGenBuffers(1, &colorBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, colorBuffer);
		glBufferData(GL_ARRAY_BUFFER, p.pc->getLength() * sizeof(float), p.pc->realVertexColors, GL_STATIC_DRAW);

		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
		glEnableVertexAttribArray(1);


		unsigned int shader = CreateShader(readFile("PassthroughVertexShader.vertexshader"), readFile("SimpleFragmentShader.fragmentshader"));
		unsigned int MatrixID = glGetUniformLocation(shader, "MVP");

		glUseProgram(shader);


		/* Loop until the user closes the window */
		while (!glfwWindowShouldClose(window))
		{
			/* Render here */
			glClear(GL_COLOR_BUFFER_BIT);

			computeMatricesFromInputs(window);
			glm::mat4 ProjectionMatrix = getProjectionMatrix();
			glm::mat4 ViewMatrix = getViewMatrix();
			glm::mat4 ModelMatrix = glm::mat4(1.0);
			glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;


			glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);

			glDrawArrays(GL_POINTS, 0, p.pc->getLength());


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