#pragma once

#include <vector>
#include <functional>

#include "Cell.h"

#include "flecs.h"

struct Grid {
	Grid(int32_t x_p, int32_t y_p);

	void unset(int32_t x, int32_t y);
	void set(int32_t x, int32_t y, flecs::entity ent);
	flecs::entity get(int32_t x, int32_t y) const;

	int32_t const size_x, size_y;
private:
	std::vector<flecs::entity> _data;
};

bool is_horizontal(CellLine const &line_p);

bool same_direction(CellLine const & a, CellLine const & b);

bool can_merge_lines(CellLine const & a, CellLine const & b);

flecs::entity merge_lines(flecs::world &ecs, flecs::entity a, flecs::entity b);

/// @brief merge all lines present in cells
void merge_cells(flecs::world &ecs, Cell & a, Cell & b);

void merge_adjacent_cells(flecs::world &ecs, Grid &grid_p, Cell &a);

void merge_all_cells(flecs::world &ecs, Grid &grid_p);
