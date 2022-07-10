#ifndef TEXTURE_HPP
#define TEXTURE_HPP

#include <GLFW/glfw3.h>

// Load a .DDS file using GLFW's own loader
GLuint loadDDS(const char * imagepath);

GLuint load_texture(const char *texture_path);

#endif
