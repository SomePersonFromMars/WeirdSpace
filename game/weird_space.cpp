// Copyright (C) 2024, Kacper Orszulak
// GNU General Public License v3.0+ (see LICENSE.txt or https://www.gnu.org/licenses/gpl-3.0.txt)

#include <cstdio>

#include "app.hpp"
#include <settings.hpp>

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
