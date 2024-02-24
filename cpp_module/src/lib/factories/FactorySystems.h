#pragma once

#include <random>
#include "flecs.h"

void create_factory_systems(flecs::world &ecs, float const &world_size_p, std::mt19937 &gen_p);
