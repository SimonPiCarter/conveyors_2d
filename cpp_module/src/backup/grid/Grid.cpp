#include "Grid.h"

#include "lib/factories/PositionedLine.h"

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

void iterate_on_positions(flecs::entity ent, std::function<void(Position const &)> &&func_p)
{
	Position const * from_l = ent.get_second<From, Position>();
	Position const * to_l = ent.get_second<To, Position>();

	if(!from_l || !to_l)
	{
		return;
	}

	bool horizontal = from_l->x != to_l->x;

	// no need to check when horizontal is true since we know that from_l->x == to_l->x
	if(horizontal && from_l->y != to_l->y)
	{
		return;
	}

	Position cur = *from_l;
	Position target = *to_l;
	int32_t offset = 1;
	if(cur.x > target.x || cur.y > target.y)
	{
		offset = -1;
	}
	while (cur.x != target.x || cur.y != target.y)
	{
		if(horizontal)
		{
			cur.x += offset;
		}
		else
		{
			cur.y += offset;
		}

		func_p(cur);
	}
}

void fill(Grid &grid_p, flecs::entity ent)
{
	iterate_on_positions(ent, [&](Position const & pos_p){
			grid_p.set(pos_p.x, pos_p.y, ent);
	});
}

bool check_line(Grid &grid_p, flecs::entity ent)
{
	bool check_ok_l = true;
	iterate_on_positions(ent, [&](Position const & pos_p){
			check_ok_l &= !grid_p.get(pos_p.x, pos_p.y);
	});
	return check_ok_l;
}

bool is_horizontal(flecs::entity ent)
{
	Position const * to_l = ent.get_second<To, Position>();
	Position const * from_l = ent.get_second<From, Position>();
	return to_l->y == from_l->y;
}

bool same_direction(flecs::entity a, flecs::entity b)
{
	return is_horizontal(a) == is_horizontal(b);
}

flecs::entity merge_on_from_pos(godot::EntityDrawer * drawer_p, Grid & grid_p, flecs::world &ecs, flecs::entity ent, Position const &pos_p)
{
	flecs::entity on_from_ent_l = grid_p.get(pos_p.x, pos_p.y);
	if(on_from_ent_l && on_from_ent_l != ent)
	{
		Position const * to_l = on_from_ent_l.get_second<To, Position>();
		if(to_l && to_l->x == pos_p.x && to_l->y == pos_p.y)
		{
			if(same_direction(on_from_ent_l, ent))
			{
				ent = set_up_merge_entity(drawer_p, grid_p, ecs, on_from_ent_l, ent);
			}
			else
			{
				create_link(ecs, "", on_from_ent_l, ent);
			}
		}
	}
	return ent;
}

flecs::entity merge_around_to_pos(godot::EntityDrawer * drawer_p, Grid & grid_p, flecs::world &ecs, flecs::entity ent, Position const &pos_p)
{
	flecs::entity up_l = grid_p.get(pos_p.x, pos_p.y - 1);
	flecs::entity down_l = grid_p.get(pos_p.x, pos_p.y + 1);
	flecs::entity left_l = grid_p.get(pos_p.x - 1, pos_p.y);
	flecs::entity right_l = grid_p.get(pos_p.x + 1, pos_p.y);
	// simple merge
	if(up_l && up_l != ent)
	{

		Position const * from_l = up_l.get_second<From, Position>();
		// if position match we merge and stop
		if(from_l && from_l->x == pos_p.x && from_l->y == pos_p.y)
		{
			if(same_direction(up_l, ent))
			{
				ent = set_up_merge_entity(drawer_p, grid_p, ecs, ent, up_l);
			}
			else
			{
				create_link(ecs, "", ent, up_l);
			}

			return ent;
		}
	}
	if(down_l && down_l != ent)
	{
		Position const * from_l = down_l.get_second<From, Position>();
		// if position match we merge and stop
		if(from_l && from_l->x == pos_p.x && from_l->y == pos_p.y)
		{
			if(same_direction(down_l, ent))
			{
				ent = set_up_merge_entity(drawer_p, grid_p, ecs, ent, down_l);
			}
			else
			{
				create_link(ecs, "", ent, down_l);
			}
			return ent;
		}
	}
	if(left_l && left_l != ent)
	{
		Position const * from_l = left_l.get_second<From, Position>();
		// if position match we merge and stop
		if(from_l && from_l->x == pos_p.x && from_l->y == pos_p.y)
		{
			if(same_direction(left_l, ent))
			{
				ent = set_up_merge_entity(drawer_p, grid_p, ecs, ent, left_l);
			}
			else
			{
				create_link(ecs, "", ent, left_l);
			}
			return ent;
		}
	}
	if(right_l && right_l != ent)
	{
		Position const * from_l = right_l.get_second<From, Position>();
		// if position match we merge and stop
		if(from_l && from_l->x == pos_p.x && from_l->y == pos_p.y)
		{
			if(same_direction(right_l, ent))
			{
				ent = set_up_merge_entity(drawer_p, grid_p, ecs, ent, right_l);
			}
			else
			{
				create_link(ecs, "", ent, right_l);
			}
			return ent;
		}
	}
	return ent;
}

flecs::entity merge_around(godot::EntityDrawer * drawer_p, Grid & grid_p, flecs::world &ecs, flecs::entity ent)
{
	if(!ent.get_second<From, Position>()
	|| !ent.get_second<To, Position>())
	{
		return ent;
	}
	Position const &pos_from_l = *ent.get_second<From, Position>();
	Position const &pos_to_l = *ent.get_second<To, Position>();

	flecs::entity new_ent_l = merge_on_from_pos(drawer_p, grid_p, ecs, ent, pos_from_l);
	return merge_around_to_pos(drawer_p, grid_p, ecs, new_ent_l, pos_to_l);
}
