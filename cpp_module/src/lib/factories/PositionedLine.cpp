#include "PositionedLine.h"

#include "lib/grid/Grid.h"
#include "lib/factories/Drawable.h"
#include "lib/factories/Storer.h"

int32_t m_length(Position const & first_p, Position const & second_p)
{
	return std::abs(first_p.x - second_p.x) + std::abs(first_p.y - second_p.y);
}

PositionedLine merge_positioned_lines(RefPositionedLine const &first_p, RefPositionedLine const &second_p)
{
	Position const & first_from_l = first_p.from;
	Position const & first_to_l = first_p.to;
	Position const & second_from_l = second_p.from;
	Position const & second_to_l = second_p.to;

	bool first_horizontal_l = first_from_l.x == first_to_l.x;
	bool second_horizontal_l = first_from_l.x == first_to_l.x;

	if(first_horizontal_l != second_horizontal_l)
	{
		return PositionedLine();
	}

	Line line_l = merge_lines(first_p.line, second_p.line);
	Position from_l = first_from_l;
	Position to_l = second_to_l;

	if (m_length(from_l, to_l) != m_length(first_from_l, first_to_l) + m_length(second_from_l, second_to_l))
	{
		return PositionedLine();
	}

	return {true, line_l, from_l, to_l, first_p.co_from, second_p.co_to};
}



flecs::entity set_up_merge_entity(godot::EntityDrawer * drawer_p, Grid & grid_p, flecs::world &ecs, flecs::entity_view first, flecs::entity_view second)
{
	// Merge
	RefPositionedLine rpl {
		*first.get<Line>(),
		*first.get_second<From, Position>(),
		*first.get_second<To, Position>(),
		first.get_second<From, Connector>()?first.get_second<From, Connector>()->ent:flecs::entity_view(),
		first.get_second<To, Connector>()?first.get_second<To, Connector>()->ent:flecs::entity_view()
	};

	RefPositionedLine rpl_second {
		*second.get<Line>(),
		*second.get_second<From, Position>(),
		*second.get_second<To, Position>(),
		second.get_second<From, Connector>()?second.get_second<From, Connector>()->ent:flecs::entity_view(),
		second.get_second<To, Connector>()?second.get_second<To, Connector>()->ent:flecs::entity_view()
	};

	PositionedLine new_pl = merge_positioned_lines(rpl, rpl_second);

	// create new merged line
	return merge_lines_entity(drawer_p, grid_p, ecs, new_pl, {first, second});
}

flecs::entity merge_lines_entity(godot::EntityDrawer * drawer_p, Grid & grid_p, flecs::world &ecs, PositionedLine const &pl, MergeLines const &ml)
{
	flecs::entity ent = ecs.entity()
		.set<From, Position>(pl.from)
		.set<To, Position>(pl.to)
		.set<Line>(pl.line);
	fill(grid_p, ent);

	// destroy items
	clean_up_line(drawer_p, ml.first);
	clean_up_line(drawer_p, ml.second);

	if(pl.co_from)
	{
		replace_connectors(ent, ml.first.mut(ecs), pl.co_from.mut(ecs));
		ent.set<From, Connector>({pl.co_from});
	}
	if(pl.co_to)
	{
		replace_connectors(ent, ml.second.mut(ecs), pl.co_to.mut(ecs));
		ent.set<To, Connector>({pl.co_to});
	}

	if(ml.first.get<Spawn>())
	{
		// copy seem necessary to avoid lost information
		Spawn new_spawn_l;
		new_spawn_l.types = ml.first.get<Spawn>()->types;
		ent.set<Spawn>(new_spawn_l);
	}

	if(ml.second.get<ConnectedToStorer>())
	{
		// copy seem necessary to avoid lost information
		ConnectedToStorer new_storer_l;
		new_storer_l.storer_ent = ml.second.get<ConnectedToStorer>()->storer_ent;
		ent.set<ConnectedToStorer>(new_storer_l);
	}

	ml.first.mut(ecs).destruct();
	ml.second.mut(ecs).destruct();

	return ent;
}
