#pragma once
#ifdef _WIN32
#include <winsock2.h>
#include <GL/gl.h>
#elif defined __linux__
#include <GL/gl.h>
#else
#define GL_SILENCE_DEPRECATION
#include <OpenGL/gl3.h>
#endif
