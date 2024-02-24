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
	if(!horizontal && from_l->y != to_l->y)
	{
		return;
	}

	Position cur = *from_l;
	Position target = *to_l;
	if(cur.x > target.x || cur.y > target.y)
	{
		std::swap(cur, target);
	}
	while (cur.x != target.x
		|| cur.y != target.y)
	{
		if(horizontal)
		{
			++cur.x;
		}
		else
		{
			++cur.y;
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

flecs::system create_merger_system(flecs::world &ecs, Grid const &grid_p)
{
	return ecs.system<flecs::pair<From, Position> const, flecs::pair<To, Position> const>("set_up_line")
		.with<FreshLine>()
		.each([&](flecs::entity& ent, flecs::pair<From, Position> const &from_p, flecs::pair<To, Position> const &to_p) {
			// Position const &pos_from_l = *from_p;
			// Position const &pos_to_l = *from_p;

			// flecs::entity up_l = grid_p.get(pos_l.x, pos_l.y - 1);
			// flecs::entity down_l = grid_p.get(pos_l.x, pos_l.y - 1);
			// flecs::entity left_l = grid_p.get(pos_l.x, pos_l.y - 1);
			// flecs::entity right_l = grid_p.get(pos_l.x, pos_l.y - 1);
			// // simple merge
			// if(up_l && !down_l)
			// {
			// 	Position const * to_l = up_l.get_second<To, Position>();
			// 	Position const * from_l = up_l.get_second<To, Position>();
			// 	// if position match we merge and stop
			// 	if(to_l && to_l->x == pos_from_l.x && to_l->y == pos_from_l.y)
			// 	{
			// 		set_up_merge_entity(ecs, up_l, ent)
			// 		return;
			// 	}
			// 	if(from_l && from_l->x == pos_to_l.x && from_l->y == pos_to_l.y)
			// 	{
			// 		set_up_merge_entity(ecs, ent, up_l)
			// 		return;
			// 	}
			// }
			// if(down_l && !up_l)
			// {
			// 	Position const * to_l = down_l.get_second<To, Position>();
			// 	Position const * from_l = down_l.get_second<To, Position>();
			// 	// if position match we merge and stop
			// 	if(to_l && to_l->x == pos_from_l.x && to_l->y == pos_from_l.y)
			// 	{
			// 		set_up_merge_entity(ecs, down_l, ent)
			// 		return;
			// 	}
			// 	if(from_l && from_l->x == pos_to_l.x && from_l->y == pos_to_l.y)
			// 	{
			// 		set_up_merge_entity(ecs, ent, down_l)
			// 		return;
			// 	}
			// }
		});
}
