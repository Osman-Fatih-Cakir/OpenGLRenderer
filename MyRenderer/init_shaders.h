#pragma once

#include <GL/glut.h>
#include <iostream>
#include <string>


std::string textFileRead(const char* filename);
void programerrors(const GLint program);
void shadererrors(const GLint shader);
GLuint initshaders(GLenum type, const char* filename);
GLuint initprogram(GLuint vertexshader, GLuint fragmentshader);
GLuint initprogram(GLuint vertexshader, GLuint geometryshader, GLuint fragmentshader);