#include <cstdio>
#include <vector>

#include <GLFW/glfw3.h>

#include <objloader.hpp>

int main(int argc, char **argv) {
	if (argc != 2) {
		fprintf(stderr, "Error: Wrong command syntax.\n");
		printf("usage: obj_dump <obj file path>\n");
		return 1;
	}

	std::vector<glm::vec3> out_vertices;
	std::vector<glm::vec2> out_uvs;
	std::vector<glm::vec3> out_normals;

	if (!loadOBJ(argv[1],
			out_vertices,
			out_uvs,
			out_normals)) {
		fprintf(stderr, "Error opening obj file \"%s\"\n", argv[1]);
		return 1;
	}

	printf("glm::vec3 out_vertices[%zu] = {\n", out_vertices.size()*3);
	for (const glm::vec3 &vec : out_vertices) {
		printf("\t%9.6f, %9.6f, %9.6f,\n", vec.x, vec.y, vec.z);
	}
	printf("};\n");

	printf("glm::vec2 out_uvs[%zu] = {\n", out_uvs.size()*2);
	for (const glm::vec2 &vec : out_uvs) {
		printf("\t%9.6f, %9.6f,\n", vec.x, vec.y);
	}
	printf("};\n");

	printf("glm::vec3 out_normals[%zu] = {\n", out_normals.size()*3);
	for (const glm::vec3 &vec : out_normals) {
		printf("\t%9.6f, %9.6f, %9.6f,\n", vec.x, vec.y, vec.z);
	}
	printf("};\n");
}
