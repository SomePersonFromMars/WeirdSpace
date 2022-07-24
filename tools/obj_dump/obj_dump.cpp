#include <cstdio>
#include <vector>

#include <GLFW/glfw3.h>

#include "objloader.hpp"

int main(int argc, char **argv) {
	if (argc != 2) {
		fprintf(stderr, "Error: Wrong command syntax.\n");
		printf("usage: obj_dump <obj file path>\n");
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

	printf("float positions[%zu] = {\n", positions.size()*3);
	for (const glm::vec3 &vec : positions) {
		printf("\t%9.6f, %9.6f, %9.6f,\n", vec.x, vec.y, vec.z);
	}
	printf("};\n");

	printf("float uvs[%zu] = {\n", uvs.size()*2);
	for (const glm::vec2 &vec : uvs) {
		printf("\t%9.6f, %9.6f,\n", vec.x, vec.y);
	}
	printf("};\n");

	printf("float normals[%zu] = {\n", normals.size()*3);
	for (const glm::vec3 &vec : normals) {
		printf("\t%9.6f, %9.6f, %9.6f,\n", vec.x, vec.y, vec.z);
	}
	printf("};\n");
}
