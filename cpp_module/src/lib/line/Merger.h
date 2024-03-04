#pragma once

#include "flecs.h"
#include "lib/line/Line.h"
#include <array>

struct Merger {
	flecs::ref<Line> out;
	std::array<flecs::ref<Line>, 2> in;
	size_t cur = 0;
};

void add_merger_system(flecs::world &ecs);
