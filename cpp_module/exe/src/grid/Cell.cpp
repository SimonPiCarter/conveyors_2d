#include "Cell.h"

#include <sstream>

bool is_eq(double a, double b, double tol)
{
	return std::abs(a-b) < tol;
}

std::ostream &operator<<(std::ostream &os_p, LinePosition const &line_p)
{
	os_p<<"["<<line_p.x<<", "<<line_p.y<<"]";
	return os_p;
}

std::ostream &operator<<(std::ostream &os_p, CellLine const &line_p)
{
	os_p<<line_p.start<<" - > " <<line_p.end;
	return os_p;
}

flecs::entity create_cell_line(flecs::world &ecs, Cell &cell_p, flecs::entity ent_cell_p, LinePosition start, LinePosition end)
{
	// create line
	CellLine line_l {start, end, {ent_cell_p}};
	std::stringstream ss_l;
	ss_l<<"line"<<start<<"-"<<end;
	flecs::entity ent_line_l = ecs.entity(ss_l.str().c_str()).set(line_l);
	// add it to the cell
	cell_p.lines.push_back(ent_line_l);
	return ent_line_l;
}

flecs::entity create_cell(flecs::world &ecs, uint32_t x, uint32_t y)
{
	std::stringstream ss_l;
	ss_l<<"cell"<<x<<"."<<y;
	return ecs.entity(ss_l.str().c_str());
}

flecs::entity create_up(flecs::world &ecs, uint32_t x, uint32_t y)
{
	flecs::entity ent_cell_l = create_cell(ecs, x, y);
	Cell cell_l {x,y};

	create_cell_line(ecs, cell_l, ent_cell_l, {double(x+0.5), double(y+1.)}, {double(x+0.5), double(y)});

	ent_cell_l.set(cell_l);
	return ent_cell_l;
}

flecs::entity create_left(flecs::world &ecs, uint32_t x, uint32_t y)
{
	flecs::entity ent_cell_l = create_cell(ecs, x, y);
	Cell cell_l {x,y};

	create_cell_line(ecs, cell_l, ent_cell_l, {double(x+1.), double(y+0.5)}, {double(x), double(y+0.5)});

	ent_cell_l.set(cell_l);
	return ent_cell_l;
}

flecs::entity create_up_left(flecs::world &ecs, uint32_t x, uint32_t y)
{
	flecs::entity ent_cell_l = create_cell(ecs, x, y);
	Cell cell_l {x,y};

	create_cell_line(ecs, cell_l, ent_cell_l, {double(x+0.5), double(y+1.)}, {double(x+0.5), double(y+0.5)});
	create_cell_line(ecs, cell_l, ent_cell_l, {double(x+0.5), double(y+0.5)}, {double(x), double(y+0.5)});

	ent_cell_l.set(cell_l);
	return ent_cell_l;
}

flecs::entity create_left_up(flecs::world &ecs, uint32_t x, uint32_t y)
{
	flecs::entity ent_cell_l = create_cell(ecs, x, y);
	Cell cell_l {x,y};

	create_cell_line(ecs, cell_l, ent_cell_l, {double(x+1.), double(y+0.5)}, {double(x+0.5), double(y+0.5)});
	create_cell_line(ecs, cell_l, ent_cell_l, {double(x+0.5), double(y+0.5)}, {double(x+0.5), double(y)});

	ent_cell_l.set(cell_l);
	return ent_cell_l;
}
