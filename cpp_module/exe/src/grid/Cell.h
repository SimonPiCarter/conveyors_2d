#pragma once

#include "flecs.h"
#include <vector>
#include <iostream>

bool is_eq(double a, double b, double tol=1e-5);

struct LinePosition {
	double x = 0;
	double y = 0;

	bool operator==(LinePosition const &other_p) const
	{
		return is_eq(x, other_p.x) && is_eq(y, other_p.y);
	}
};

std::ostream &operator<<(std::ostream &os_p, LinePosition const &line_p);

struct CellLine {
	LinePosition start;
	LinePosition end;
	std::vector<flecs::entity> cells;
};

std::ostream &operator<<(std::ostream &os_p, CellLine const &line_p);

struct Cell {
	uint32_t x = 0;
	uint32_t y = 0;
	std::vector<flecs::entity> lines;
};

flecs::entity create_up(flecs::world &ecs, uint32_t x, uint32_t y);
flecs::entity create_left(flecs::world &ecs, uint32_t x, uint32_t y);
flecs::entity create_up_left(flecs::world &ecs, uint32_t x, uint32_t y);
flecs::entity create_left_up(flecs::world &ecs, uint32_t x, uint32_t y);
