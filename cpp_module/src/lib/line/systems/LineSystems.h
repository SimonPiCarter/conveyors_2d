#pragma once

#include <random>
#include "flecs.h"

#include "lib/line/Line.h"

void set_up_line_systems(flecs::world &ecs, uint32_t const &timestamp_p, float const &world_size_p, std::mt19937 &gen_p, bool print_p);
