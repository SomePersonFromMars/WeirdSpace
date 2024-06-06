// Copyright (C) 2024, Kacper Orszulak
// GNU General Public License v3.0+ (see LICENSE.txt or https://www.gnu.org/licenses/gpl-3.0.txt)

#ifndef OBJLOADER_H
#define OBJLOADER_H

#include <vector>

#include <glm/glm.hpp>

// Credits: https://www.opengl-tutorial.org/beginners-tutorials/tutorial-7-model-loading/

bool loadOBJ(
	const char * path,
	std::vector<glm::vec3> & out_vertices,
	std::vector<glm::vec2> & out_uvs,
	std::vector<glm::vec3> & out_normals
);

#endif
