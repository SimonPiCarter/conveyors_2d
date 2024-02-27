#include "Line.h"

#include "Connector.h"

Line::Line(uint32_t capacity_p) :
	first(size_t(capacity_p)), dist_start(capacity_p*100), dist_end(0), full_dist(capacity_p*100)
{
	items.resize(capacity_p);
	for(size_t i = 0 ; i < capacity_p; ++i)
	{
		free_idx.push_back(i);
	}
}

void step(Line &line_p)
{
	if(get_content_size(line_p) == 0)
	{
		return;
	}

	int32_t remaining_movement = 0;
	if (line_p.dist_end < line_p.speed)
	{
		remaining_movement = line_p.speed - line_p.dist_end;
		line_p.dist_end = 0;
	}
	else
	{
		line_p.dist_end -= line_p.speed;
	}

	size_t last = line_p.first;
	size_t idx = line_p.first;
	while(remaining_movement > 0 && idx < line_p.items.size())
	{
		ItemOnLine &item_l = line_p.items[idx];
		if(remaining_movement > item_l.dist_to_next)
		{
			remaining_movement = remaining_movement - item_l.dist_to_next;
			item_l.dist_to_next = 0;
		}
		else
		{
			item_l.dist_to_next -= remaining_movement;
			remaining_movement = 0;
		}
		last = idx;
		idx = item_l.next;
	}

	unsigned long performed_movement = line_p.speed - remaining_movement;
	line_p.dist_start += performed_movement;
}
void empty_line(Line &line_p)
{
	line_p.free_idx.clear();
	for(size_t i = 0 ; i < line_p.items.size(); ++i)
	{
		line_p.free_idx.push_back(i);
	}
	line_p.first = line_p.items.size();
	line_p.dist_start = line_p.full_dist;
	line_p.dist_end = 0;
}

bool can_add(Line const &line_p)
{
	if(line_p.dist_start < 100 || line_p.free_idx.size() == 0)
	{
		return false;
	}
	return true;
}

flecs::entity_view can_consume(Line const &line_p)
{
	if(line_p.dist_end > 0 || is_empty(line_p))
	{
		return flecs::entity();
	}
	ItemOnLine const & item_l = line_p.items[line_p.first];
	return item_l.ent;
}

bool add_to_start(Line &line_p, flecs::entity_view const &item_p)
{
	if(!can_add(line_p))
	{
		return false;
	}

	size_t idx_l = line_p.free_idx.front();
	line_p.free_idx.pop_front();

	if(is_empty(line_p))
	{
		line_p.first = idx_l;
		line_p.dist_end = line_p.full_dist - 100;
	}

	// update chaining
	line_p.items[line_p.last].next = idx_l;
	line_p.items[line_p.last].dist_to_next = line_p.dist_start - 100;
	line_p.last = idx_l;

	// init item
	line_p.items[idx_l].ent = item_p;
	line_p.items[idx_l].next = line_p.items.size();
	line_p.items[idx_l].dist_to_next = 0;

	// update distance
	line_p.dist_start = 0;

	return true;
}

flecs::entity_view consume(Line &line_p)
{
	if(!can_consume(line_p))
	{
		return flecs::entity();
	}

	ItemOnLine const & item_l = line_p.items[line_p.first];
	line_p.free_idx.push_back(line_p.first);
	line_p.first = item_l.next;
	line_p.dist_end = item_l.dist_to_next + 100;

	if(is_empty(line_p))
	{
		line_p.dist_start = line_p.full_dist;
	}

	return item_l.ent;
}

size_t get_content_size(Line const &line_p)
{
	return line_p.items.size() - line_p.free_idx.size();
}

size_t get_size(Line const &line_p)
{
	return line_p.items.size();
}

bool is_empty(Line const &line_p)
{
	return line_p.first >= line_p.items.size();
}

Line merge_lines(Line const &first_p, Line const &second_p)
{
	return Line(get_size(first_p) + get_size(second_p));
}

flecs::entity create_link(flecs::world &ecs, std::string const &, flecs::entity &from_p, flecs::entity &to_p)
{
	Line const *line_to_l = to_p.get<Line>();
	Position const *pos_from_l = to_p.get<From, Position>();
	Position const *pos_to_l = to_p.get<To, Position>();
	int32_t unitary_x_l = (pos_to_l->x - pos_from_l->x) / int32_t(get_size(*line_to_l));
	int32_t unitary_y_l = (pos_to_l->y - pos_from_l->y) / int32_t(get_size(*line_to_l));
	Position link_from_l = *pos_from_l;
	Position link_to_l = *pos_from_l;
	link_from_l.x -= unitary_x_l;
	link_from_l.y -= unitary_y_l;
	link_to_l.x += unitary_x_l;
	link_to_l.y += unitary_y_l;
	Line line_l(2);
	flecs::entity link_l = ecs.entity()
		.set<From, Position>(link_from_l)
		.set<To, Position>(link_to_l)
		.set<Line>(line_l)
		.set<Input>({from_p.get_ref<Line>()})
		.set<Output>({to_p.get_ref<Line>()});

	from_p.set<To, Connector>({link_l});
	to_p.set<From, Connector>({link_l});

	return link_l;
}
