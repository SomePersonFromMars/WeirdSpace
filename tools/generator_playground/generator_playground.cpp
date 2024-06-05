#include <cstdio>

#include "app.hpp"
#include <useful.hpp>

int main() {
    do {
        if (global_settings.is_global_reload_pending()) {
            global_settings.mark_global_reload_completed();
            printf("\n--- Performing a hard reload... ---\n");
        }

        app_t app;
        app.init();
        app.loop();
        app.deinit();

    } while (global_settings.is_global_reload_pending());

	return 0;
}
