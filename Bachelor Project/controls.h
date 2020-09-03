#ifndef CONTROLS_HPP
#define CONTROLS_HPP
#include "glm/glm.hpp"
#include <GLFW/glfw3.h>
void computeMatricesFromInputs(GLFWwindow* window);
glm::mat4 getViewMatrix();
glm::mat4 getProjectionMatrix();
#endif