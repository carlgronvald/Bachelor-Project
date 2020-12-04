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
int getPointSize();
float getkdt();
float getkd();
float getkt();
float getkc();
float getsigma2();
float getkdist();
void setExtFOV(float fov);
void setPointSize(int pointSize);
void setkdt(float kdt);
void setkd(float kd);
void setkt(float kt);
void setkc(float kc);
void setsigma2(float sigma2);
void setkdist(float kdist);
#endif