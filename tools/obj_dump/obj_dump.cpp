// Copyright (C) 2024, Kacper Orszulak
// GNU General Public License v3.0+ (see LICENSE.txt or https://www.gnu.org/licenses/gpl-3.0.txt)

#include <cstdio>
#include <cstring>
#include <vector>

#include <GLFW/glfw3.h>

#include "objloader.hpp"

int main(int argc, char **argv) {
	bool format_as_vec4 = argc == 3 && strcmp(argv[2], "vec4") == 0;
	if (!(argc == 2 || format_as_vec4)) {
		fprintf(stderr, "Error: Wrong command syntax.\n");
		printf("usage: obj_dump <obj file path> [vec4]\n");
		return 1;
	}

	std::vector<glm::vec3> positions;
	std::vector<glm::vec2> uvs;
	std::vector<glm::vec3> normals;

	if (!loadOBJ(argv[1],
			positions,
			uvs,
			normals)) {
		fprintf(stderr, "Error opening obj file \"%s\"\n", argv[1]);
		return 1;
	}

	printf("float positions[%zu] = {\n",
			format_as_vec4 ? positions.size()*4 : positions.size()*3);
	for (const glm::vec3 &vec : positions) {
		if (!format_as_vec4)
			printf("\t%9.6f, %9.6f, %9.6f,\n", vec.x, vec.y, vec.z);
		else
			printf("\t%9.6f, %9.6f, %9.6f, %9.6f,\n",
					vec.x, vec.y, vec.z, 0.0f);
	}
	printf("};\n");

	printf("float uvs[%zu] = {\n",
			format_as_vec4 ? uvs.size()*4 : uvs.size()*2);
	for (const glm::vec2 &vec : uvs) {
		if (!format_as_vec4)
			printf("\t%9.6f, %9.6f,\n", vec.x, vec.y);
		else
			printf("\t%9.6f, %9.6f, %9.6f, %9.6f,\n",
					vec.x, vec.y, 0.0f, 0.0f);
	}
	printf("};\n");

	printf("float normals[%zu] = {\n",
			format_as_vec4 ? normals.size()*4 : normals.size()*3);
	for (const glm::vec3 &vec : normals) {
		if (!format_as_vec4)
			printf("\t%9.6f, %9.6f, %9.6f,\n", vec.x, vec.y, vec.z);
		else
			printf("\t%9.6f, %9.6f, %9.6f, %9.6f,\n",
					vec.x, vec.y, vec.z, 0.0f);
	}
	printf("};\n");
}
