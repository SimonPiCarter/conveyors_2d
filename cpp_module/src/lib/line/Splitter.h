#pragma once

#include "flecs.h"
#include "lib/line/Line.h"
#include <array>

struct Splitter {
	flecs::ref<Line> in;
	std::array<flecs::ref<Line>, 2> out;
	size_t cur = 0;
};

void add_splitter_system(flecs::world &ecs);
