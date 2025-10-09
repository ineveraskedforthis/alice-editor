#pragma once
#include "GL/glew.h"
#include <string>
#include <iostream>

namespace ogl {
void inline check_gl_error(std::string message) {
	auto error = glGetError();
	if (error != GL_NO_ERROR) {
		std::cout << "Error at ";
		std::cout << message << "\n";
		std::cout << "Gl Error " << error << ": ";
		std::cout << glewGetErrorString(error) << "\n";
	}
}
};