
#pragma once

#include "Connector.h"
#include "Line.h"

namespace godot
{
	class EntityDrawer;
}

struct Grid;

struct RefPositionedLine
{
	Line const&line;
	Position const&from;
	Position const&to;
	flecs::entity_view co_from;
	flecs::entity_view co_to;
};

struct PositionedLine
{
	bool valid = false;
	Line line;
	Position from;
	Position to;
	flecs::entity_view co_from;
	flecs::entity_view co_to;
};

struct MergeLines
{
	flecs::entity_view first;
	flecs::entity_view second;
};

PositionedLine merge_positioned_lines(RefPositionedLine const &first_p, RefPositionedLine const &second_p);

flecs::entity set_up_merge_entity(godot::EntityDrawer * drawer_p, Grid & grid_p, flecs::world &ecs, flecs::entity_view first, flecs::entity_view second);

flecs::entity merge_lines_entity(godot::EntityDrawer * drawer_p, Grid & grid_p, flecs::world &ecs, PositionedLine const &pl, MergeLines const &ml);
