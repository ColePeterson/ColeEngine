#pragma once

#ifndef _PLATFORM
#define _PLATFORM

#define GLFW_INCLUDE_NONE

#include "glew.h"

#include <GLFW/glfw3.h>

//double mouseX, mouseY;
//bool mouseLeft, mouseRight;

class Platform
{
public:
	Platform(int _width, int _height);
	~Platform();

	void setWindowSize(int _width, int _height);

	void update();
	

	int width, height;

	GLFWwindow* window;


	double mouseX, mouseY, dx, dy;
	bool mouseLeft, mouseRight, mouseLeftRelease, mouseRightRelease;
	bool s_down, w_down, d_down, a_down, q_down, lshift_down;

private:
	void updateMouse();
	void updateKeyboard();
	
	bool focused, menu;
};

#endif