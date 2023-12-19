///////////////////////////////////////////////////////////////////////
// A slight encapsulation of a shader program. This contains methods
// to build a shader program from multiples files containing vertex
// and pixel shader code, and a method to link the result.  When
// loaded (method "Use"), its vertex shader and pixel shader will be
// invoked for all geometry passing through the graphics pipeline.
// When done, unload it with method "Unuse".
////////////////////////////////////////////////////////////////////////

#include <fstream>
#include <iostream>


#include "Shader.h"
#include "Logging.h"

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
// Reads a specified file into a string and returns the string.  The
// file is examined first to determine the needed string size.
char* ReadFile(const char* name)
{
    std::ifstream f;
    f.open(name, std::ios_base::binary); // Open
    f.seekg(0, std::ios_base::end);      // Position at end
    int length = f.tellg();              //   to get the length

    char* content = new char[length + 1]; // Create buffer of needed length
    f.seekg(0, std::ios_base::beg);     // Position at beginning
    f.read(content, length);            //   to read complete file
    f.close();                           // Close

   
    content[length] = char(0);           // Finish with a NULL
    return content;
}

// Creates an empty shader program.
ShaderProgram::ShaderProgram(std::string _name)
    : name(name)
{
    programId = glCreateProgram();
    //LOG_GL_ERROR();
}

// Use a shader program
void ShaderProgram::UseShader()
{
    glUseProgram(programId);
    //LOG_GL_ERROR();
}

// Done using a shader program
void ShaderProgram::UnuseShader()
{
    glUseProgram(0);
    //LOG_GL_ERROR();
}

// Read, send to OpenGL, and compile a single file into a shader
// program.  In case of an error, retrieve and print the error log
// string.
void ShaderProgram::AddShader(std::string fileName, GLenum type)
{
    // Read the source from the named file
    char* src = ReadFile(fileName.c_str());
    const char* psrc[1] = { src };

    shaderSrc = std::string(src);

    // Create a shader and attach, hand it the source, and compile it.
    int shader = glCreateShader(type);
    glAttachShader(programId, shader);

    //LOG_GL_ERROR();

    glShaderSource(shader, 1, psrc, NULL);

   // LOG_GL_ERROR();

    glCompileShader(shader);

    //LOG_GL_ERROR();

    delete src;

    // Get the compilation status
    int status;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);

    
    // If compilation status is not OK, get and print the log message.
    if (status != 1) 
    {
        int length;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
        char* buffer = new char[length];
        glGetShaderInfoLog(shader, length, NULL, buffer);
        printf("Compile log for %s:\n%s\n", fileName.c_str(), buffer);
        delete buffer;
    }
}

// Link a shader program after all the shader files have been added
// with the AddShader method.  In case of an error, retrieve and print
// the error log string.
void ShaderProgram::LinkProgram()
{
    // Link program and check the status
    glLinkProgram(programId);
    int status;
    glGetProgramiv(programId, GL_LINK_STATUS, &status);

    //LOG_GL_ERROR();

    // If link failed, get and print log
    if (status != 1) 
    {
        int length;
        glGetProgramiv(programId, GL_INFO_LOG_LENGTH, &length);
        char* buffer = new char[length];
        glGetProgramInfoLog(programId, length, NULL, buffer);
        printf("Link log:\n%s\n", buffer);
        delete buffer;
    }
}
