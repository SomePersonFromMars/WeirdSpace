#include "app.hpp"
#include "useful.hpp"

int main() {
	app_t app;

	app.init();

	GLint max_texture_size;
	GLint max_compute_work_group_count[3];
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max_texture_size);
	glGetIntegeri_v(
			GL_MAX_COMPUTE_WORK_GROUP_COUNT,
			0, &max_compute_work_group_count[0]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT,
			1, &max_compute_work_group_count[1]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT,
			2, &max_compute_work_group_count[2]);
	PRINT_D(max_texture_size);
	PRINT_D(max_compute_work_group_count[0]);
	PRINT_D(max_compute_work_group_count[1]);
	PRINT_D(max_compute_work_group_count[2]);

	app.loop();
	app.deinit();

	return 0;
}
