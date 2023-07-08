#include "app.hpp"
#include <useful.hpp>

int main() {
	app_t app;

	app.init();
	app.loop();
	app.deinit();

	return 0;
}
