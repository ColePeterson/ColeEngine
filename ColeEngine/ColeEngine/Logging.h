#pragma once

#ifndef _LOGGING
#define _LOGGING


#include <string>
#include <iostream>
#include "glew.h"

/*
#define LOG_GL_ERROR() logGlError(__FILE__, __LINE__)


void logGlError(const char* file, int line) {
	GLenum error = glGetError();
	if (error != GL_NO_ERROR) {
		const char* errorString = reinterpret_cast<const char*>(gluErrorString(error));
		std::cerr << "OpenGL Error (" << file << ":" << line << "): " << errorString << std::endl;
	}
}
*/

class Log
{
public:
	Log() = delete;

	
	static void init();
	static void msg(std::string msg);
	static void warning(std::string msg);
	static void error(std::string msg);
	static void fatal(std::string msg);
	static void info(std::string msg);
	void logGlError(const char* file, int line);

};

#endif