#ifndef CONTROLS_HPP
#define CONTROLS_HPP
#include "glm/glm.hpp"
#include <GLFW/glfw3.h>
void computeMatricesFromInputs(GLFWwindow* window);
glm::mat4 getViewMatrix();
glm::mat4 getProjectionMatrix();
glm::vec3 getPosition();
glm::vec3 getDirection();
glm::vec2 getAngles();
float getExtFOV();
float getPointSize();
float getkdt();
float getkd();
float getkt();
float getkc();
#endif