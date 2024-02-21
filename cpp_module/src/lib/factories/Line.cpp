#include "Line.h"

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

bool is_empty(Line const &line_p)
{
	return line_p.first >= line_p.items.size();
}
