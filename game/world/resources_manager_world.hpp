// Copyright (C) 2024, Kacper Orszulak
// GNU General Public License v3.0+ (see LICENSE.txt or https://www.gnu.org/licenses/gpl-3.0.txt)

#pragma once
#ifndef RESOURCES_MANAGER_WORLD_HPP
#define RESOURCES_MANAGER_WORLD_HPP

#include <utility>
#include <vector>
#include <map>

#include <utilities/expiration_queue.hpp>
#include "world_buffer.hpp"

class resources_manager_world_t {
    public:
        // Methods
        resources_manager_world_t(world_buffer_t &world_buffer);

        void refresh_preprocessing_tasks(const camera_t &camera);
        bool preprocess_most_urgent_chunk();

    private:
        // Data
        world_buffer_t &world_buffer;
};

#endif
