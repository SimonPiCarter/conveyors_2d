#pragma once

#include <vector>
#include <functional>

#include "flecs.h"
#include "lib/factories/Line.h"

struct Grid
{
	Grid(int32_t x_p, int32_t y_p);

	void unset(int32_t x, int32_t y);
	void set(int32_t x, int32_t y, flecs::entity ent);
	flecs::entity get(int32_t x, int32_t y) const;

	int32_t const size_x, size_y;
private:
	std::vector<flecs::entity> _data;
};

void iterate_on_positions(flecs::entity ent, std::function<void(Position const &)> &&func_p);

void fill(Grid &grid_p, flecs::entity ent);

flecs::system create_merger_system(flecs::world &ecs, Grid const &grid_p);