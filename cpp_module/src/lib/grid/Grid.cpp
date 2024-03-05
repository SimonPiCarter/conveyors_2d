#include "Grid.h"

#include <iostream>
#include <sstream>

#include "lib/line/Line.h"
#include "lib/line/Merger.h"
#include "lib/line/Splitter.h"
#include "lib/line/storer/Storer.h"

Grid::Grid(int32_t x_p, int32_t y_p)
	: size_x(x_p), size_y(y_p), _data(x_p*y_p, flecs::entity())
{}

void Grid::unset(int32_t x, int32_t y)
{
	if(x*size_y+y < 0 || x*size_y+y >= _data.size())
	{
		return;
	}
	_data[x*size_y+y] = flecs::entity();
}

void Grid::set(int32_t x, int32_t y, flecs::entity ent)
{
	if(x*size_y+y < 0 || x*size_y+y >= _data.size())
	{
		return;
	}
	_data[x*size_y+y] = ent;
}

flecs::entity Grid::get(int32_t x, int32_t y) const
{
	if(x*size_y+y < 0 || x*size_y+y >= _data.size())
	{
		return flecs::entity();
	}
	return _data[x*size_y+y];
}

bool is_horizontal(CellLine const &line_p)
{
	return is_eq(line_p.start.y, line_p.end.y);
}

bool same_direction(CellLine const & a, CellLine const & b)
{
	return is_horizontal(a) == is_horizontal(b);
}

bool is_middle_point(LinePosition const &point)
{
	return (std::ceil(point.x) - point.x) + (std::ceil(point.y) - point.y) > 0.95;
}

bool can_merge_lines(CellLine const & a, CellLine const & b)
{
	if(!same_direction(a,b))
	{
		return false;
	}
	return (!is_middle_point(a.start) && a.start == b.end)
		|| (!is_middle_point(b.start) && b.start == a.end);
}

flecs::entity merge_lines(flecs::world &ecs, flecs::entity a, flecs::entity b)
{
	CellLine const *line_a = a.get<CellLine>();
	CellLine const *line_b = b.get<CellLine>();
	if(!line_a || !line_b)
	{
		return a;
	}
	if(!can_merge_lines(*line_a, *line_b))
	{
		return a;
	}

	CellLine const &first = line_a->end == line_b->start? *line_a : *line_b;
	CellLine const &second = line_a->end == line_b->start? *line_b : *line_a;

	CellLine line {first.start, second.end};

	line.cells.insert(line.cells.end(), first.cells.begin(), first.cells.end());
	line.cells.insert(line.cells.end(), second.cells.begin(), second.cells.end());

	std::stringstream ss_l;
	ss_l<<"line"<<first.start<<"-"<<second.end;
	flecs::entity new_line = ecs.entity(ss_l.str().c_str());

	for(flecs::entity ent : line.cells)
	{
		Cell * cell_l = ent.get_mut<Cell>();
		if(cell_l)
		{
			// remove old lines
			if(std::find(cell_l->lines.begin(), cell_l->lines.end(), a) != cell_l->lines.end())
				cell_l->lines.erase(std::find(cell_l->lines.begin(), cell_l->lines.end(), a));
			if(std::find(cell_l->lines.begin(), cell_l->lines.end(), b) != cell_l->lines.end())
				cell_l->lines.erase(std::find(cell_l->lines.begin(), cell_l->lines.end(), b));
			// add new line
			cell_l->lines.push_back(new_line);
		}
	}

	new_line.set(line);

	if(a.get<Spawn>()) {
		Spawn copy_l = *a.get<Spawn>();
		new_line.set(copy_l);
	}
	else if(b.get<Spawn>()) {
		Spawn copy_l = *b.get<Spawn>();
		new_line.set(copy_l);
	}

	if(a.get<ConnectedToStorer>()) {
		ConnectedToStorer copy_l = *a.get<ConnectedToStorer>();
		new_line.set(copy_l);
	}
	else if(b.get<ConnectedToStorer>()) {
		ConnectedToStorer copy_l = *b.get<ConnectedToStorer>();
		new_line.set(copy_l);
	}

	a.destruct();
	b.destruct();

	return new_line;
}

/// @brief merge all lines present in cells
void merge_cells(flecs::world &ecs, Cell & a, Cell & b)
{
	for(flecs::entity line_a : a.lines)
	{
		for(flecs::entity line_b : b.lines)
		{
			merge_lines(ecs, line_a, line_b);
		}
	}
}

void merge_adjacent_cells(flecs::world &ecs, Grid &grid_p, Cell &a)
{
	flecs::entity adjacent_cell;
	adjacent_cell = grid_p.get(a.x-1, a.y);
	if(adjacent_cell && adjacent_cell.get<Cell>())
	{
		merge_cells(ecs, a, *adjacent_cell.get_mut<Cell>());
	}
	adjacent_cell = grid_p.get(a.x+1, a.y);
	if(adjacent_cell && adjacent_cell.get<Cell>())
	{
		merge_cells(ecs, a, *adjacent_cell.get_mut<Cell>());
	}
	adjacent_cell = grid_p.get(a.x, a.y-1);
	if(adjacent_cell && adjacent_cell.get<Cell>())
	{
		merge_cells(ecs, a, *adjacent_cell.get_mut<Cell>());
	}
	adjacent_cell = grid_p.get(a.x, a.y+1);
	if(adjacent_cell && adjacent_cell.get<Cell>())
	{
		merge_cells(ecs, a, *adjacent_cell.get_mut<Cell>());
	}
}

void merge_all_cells(flecs::world &ecs, Grid &grid_p)
{
	ecs.filter<Cell>()
		.each([&](flecs::entity e, Cell &cell){
			merge_adjacent_cells(ecs, grid_p, cell);
		});
}

void link_all_simple_cells(flecs::world &ecs)
{
	ecs.defer([&]{
		ecs.filter<Cell>()
		.each([&](flecs::entity e, Cell &cell){
			if(cell.lines.size() == 2
			&& cell.lines[0] && cell.lines[0].get<CellLine>()
			&& cell.lines[1] && cell.lines[1].get<CellLine>())
			{
				flecs::entity line0 = cell.lines[0];
				flecs::entity line1 = cell.lines[1];

				CellLine const &pos0_l = *cell.lines[0].get<CellLine>();
				CellLine const &pos1_l = *cell.lines[1].get<CellLine>();

				if(pos0_l.end == pos1_l.start)
				{
					connect(line0, line1);
				}
				if(pos1_l.end == pos0_l.start)
				{
					connect(line1, line0);
				}
			}
		});
	});
}

void link_all_splitter_cells(flecs::world &ecs)
{
	ecs.defer([&]{
		ecs.filter<Cell>()
		.each([&](flecs::entity e, Cell &cell){
			if(cell.lines.size() == 3
			&& cell.lines[0] && cell.lines[0].get<CellLine>()
			&& cell.lines[1] && cell.lines[1].get<CellLine>()
			&& cell.lines[2] && cell.lines[2].get<CellLine>())
			{
				flecs::entity line0 = cell.lines[0];
				flecs::entity line1 = cell.lines[1];
				flecs::entity line2 = cell.lines[2];

				CellLine const &pos0_l = *cell.lines[0].get<CellLine>();
				CellLine const &pos1_l = *cell.lines[1].get<CellLine>();
				CellLine const &pos2_l = *cell.lines[2].get<CellLine>();
				Splitter splitter;
				bool not_found = false;
				if(pos0_l.end == pos1_l.start
				&& pos0_l.end == pos2_l.start)
				{
					splitter = {line1.get_ref<Line>(), line2.get_ref<Line>()};
				}
				else if(pos1_l.end == pos0_l.start
					 && pos1_l.end == pos2_l.start)
				{
					splitter = {line1.get_ref<Line>(), {line0.get_ref<Line>(), line2.get_ref<Line>()}};
				}
				else if(pos2_l.end == pos0_l.start
					 && pos2_l.end == pos1_l.start)
				{
					splitter = {line2.get_ref<Line>(), {line0.get_ref<Line>(), line1.get_ref<Line>()}};
				}
				else
				{
					not_found = true;
				}
				if(!not_found)
				{
					// make output ignore performed movement
					// this fasten the insertion on line and
					// avoid slowing down
					if(splitter.out[0].try_get()) {
						splitter.out[0].try_get()->ignore_performed_movement = true;
					}
					if(splitter.out[1].try_get()) {
						splitter.out[1].try_get()->ignore_performed_movement = true;
					}
					if(splitter.in.try_get()) {
						splitter.in.try_get()->ignore_next_dist = true;
					}
					e.set<Splitter>(splitter);
				}
			}
		});
	});
}

void link_all_merger_cells(flecs::world &ecs)
{
	ecs.defer([&]{
		ecs.filter<Cell>()
		.each([&](flecs::entity e, Cell &cell){
			if(cell.lines.size() == 3
			&& cell.lines[0] && cell.lines[0].get<CellLine>()
			&& cell.lines[1] && cell.lines[1].get<CellLine>()
			&& cell.lines[2] && cell.lines[2].get<CellLine>())
			{
				flecs::entity line0 = cell.lines[0];
				flecs::entity line1 = cell.lines[1];
				flecs::entity line2 = cell.lines[2];

				CellLine const &pos0_l = *cell.lines[0].get<CellLine>();
				CellLine const &pos1_l = *cell.lines[1].get<CellLine>();
				CellLine const &pos2_l = *cell.lines[2].get<CellLine>();
				Merger merger;
				bool not_found = false;
				if(pos0_l.start == pos1_l.end
				&& pos0_l.start == pos2_l.end)
				{
					merger = {line0.get_ref<Line>(), {line1.get_ref<Line>(), line2.get_ref<Line>()}};
				}
				else if(pos1_l.start == pos0_l.end
					 && pos1_l.start == pos2_l.end)
				{
					merger = {line1.get_ref<Line>(), {line0.get_ref<Line>(), line2.get_ref<Line>()}};
				}
				else if(pos2_l.start == pos0_l.end
					 && pos2_l.start == pos1_l.end)
				{
					merger = {line2.get_ref<Line>(), {line0.get_ref<Line>(), line1.get_ref<Line>()}};
				}
				else
				{
					not_found = true;
				}
				if(!not_found)
				{
					// make output ignore performed movement
					// this fasten the insertion on line and
					// avoid slowing down
					if(merger.out.try_get()) {
						merger.out.try_get()->ignore_performed_movement = true;
					}
					e.set<Merger>(merger);
				}
			}
		});
	});
}
