#include "Line.h"

#include <iostream>

bool is_empty(Line const &line_p)
{
	return line_p.first >= line_p.items.size();
}

bool can_add(Line &line_p)
{
	if(line_p.dist_start < 100 || line_p.free_idx.size() == 0)
	{
		return false;
	}
	return true;
}

bool neo_add_to_start(Line &line_p, flecs::entity_view const &item_p, uint32_t movement_p)
{
	if(!can_add(line_p))
	{
		return false;
	}

	size_t idx_l = line_p.free_idx.front();
	line_p.free_idx.pop_front();

	// cap movement
	if(is_empty(line_p))
	{
		// cap movement
		uint32_t capped_move = std::min(line_p.dist_start, movement_p);
		line_p.first = idx_l;
		line_p.dist_end = line_p.full_dist - capped_move;
	}
	else
	{
		// cap movement further
		uint32_t capped_move = std::min(line_p.dist_start - 100, movement_p);
		// update chaining
		line_p.items[line_p.last].next = idx_l;
		line_p.items[line_p.last].dist_to_next = line_p.dist_start - movement_p;
	}
	line_p.last = idx_l;

	// init item
	line_p.items[idx_l].ent = item_p;
	line_p.items[idx_l].next = line_p.items.size();
	line_p.items[idx_l].dist_to_next = 0;

	// update distance
	line_p.dist_start = movement_p;

	return true;
}

int32_t get_max_movement(Line const &line_p, size_t last_p, size_t cur_p)
{
	if(last_p >= line_p.items.size())
	{
		if(!line_p.next_line)
		{
			return line_p.dist_end;
		}
		int32_t max_movement = line_p.next_line->dist_start + line_p.dist_end;
		if(!is_empty(*line_p.next_line))
		{
			max_movement = std::max(0, max_movement - 100);
		}
		return max_movement;
	}
	else
	{
		ItemOnLine const &item = line_p.items[last_p];
		return item.dist_to_next - 100;
	}
	return 0;
}

flecs::entity_view remove_first_from_line(Line &line_p)
{
	if(is_empty(line_p))
	{
		return flecs::entity();
	}

	ItemOnLine const & item_l = line_p.items[line_p.first];
	line_p.free_idx.push_back(line_p.first);
	line_p.first = item_l.next;
	line_p.dist_end += item_l.dist_to_next;

	if(is_empty(line_p))
	{
		line_p.dist_start = line_p.full_dist;
	}

	return item_l.ent;
}

void neo_step(Line &line_p)
{
	line_p.sent_to_next = false;
	if(is_empty(line_p))
	{
		return;
	}

	int32_t remaining_movement = 0;
	int32_t move = line_p.speed;
	size_t last = line_p.items.size();
	size_t cur = line_p.first;

	// while we have remaining movement to be handled
	while(move > 0 && cur < line_p.items.size())
	{
		int32_t max_movement = get_max_movement(line_p, last, cur);
		if(max_movement < move)
		{
			remaining_movement = move - max_movement;
			move = max_movement;
		}
		// get the current item on the line
		ItemOnLine &item_l = line_p.items[cur];
		// if there is an element before this item
		// we reduce the distance to this element
		// max movement
		if(last < line_p.items.size())
		{
			item_l.dist_to_next -= move;
			move = remaining_movement;
			last = cur;
			cur = item_l.next;
		}
		// if this is the first element of the line
		// and there is enough room on the line
		else if(move <= line_p.dist_end)
		{
			line_p.dist_end -= move;
			move = remaining_movement;
			last = cur;
			cur = item_l.next;
		}
		// there is not enough room on the line
		// but we know there is enough room
		// on the next one
		else if(line_p.next_line)
		{
			neo_add_to_start(*line_p.next_line, item_l.ent, move - line_p.dist_end);
			remove_first_from_line(line_p);
			// do not update remaining move!
			last = line_p.items.size();
			cur = line_p.first;
			line_p.sent_to_next = true;
		}
	}

	unsigned long performed_movement = line_p.speed - remaining_movement;
	// update the distance to start based on the total movement of the last item entered in the line
	line_p.dist_start += performed_movement;
}

void for_each_item(Line const &line_p, std::function<void(ItemOnLine const &, int32_t)> const &func_p)
{
	size_t cur = line_p.first;
	int32_t pos = line_p.dist_end;
	while(cur < line_p.items.size())
	{
		func_p(line_p.items[cur], pos);
		pos += line_p.items[cur].dist_to_next;
		cur = line_p.items[cur].next;
	}
}


void tag_magnitude(flecs::entity ent, std::vector<flecs::entity> &nexts_out) {
	if(ent.get<OutputLine>())
	{
		nexts_out.insert(nexts_out.end(), ent.get<OutputLine>()->lines.begin(), ent.get<OutputLine>()->lines.end());
	}
}

void tag_all_magnitude(flecs::world &ecs) {

	ecs.defer([&]{
		ecs.filter<Line const>().each([&](flecs::entity ent, Line const &){
			ent.set<Magnitude>({0});
		});
	});

	ecs.filter<Spawn const>().each([&](flecs::entity ent, Spawn const &){
		std::set<flecs::entity> set_tagged_l;
		Magnitude cur = {0};
		uint32_t max_delayed_magnitude = 0;
		std::map<uint32_t, std::vector<flecs::entity> > delayed_magnitude;
		std::vector<flecs::entity> cur_magnitude = {ent};
		while(!cur_magnitude.empty() || cur.order < max_delayed_magnitude)
		{
			std::vector<flecs::entity> next_magnitude = delayed_magnitude[cur.order];
			for(flecs::entity ent : cur_magnitude) {

				Magnitude * ent_magnitude = ent.get_mut<Magnitude>();
				if(!ent_magnitude) {
					std::cerr<<"error no magnitude"<<std::endl;
					continue;
				}

				if(ent_magnitude->order > cur.order) {
					delayed_magnitude[ent_magnitude->order].push_back(ent);
					max_delayed_magnitude = std::max(max_delayed_magnitude, ent_magnitude->order);
				}
				else if(set_tagged_l.insert(ent).second) {
					ent_magnitude->order = cur.order;
					tag_magnitude(ent, next_magnitude);
				}
			}
			++cur.order;
			std::swap(cur_magnitude, next_magnitude);
		}
	});
}

void connect(flecs::entity from, flecs::entity to)
{
	if(!from.get<OutputLine>())
	{
		from.set<OutputLine>({{to}});
	}
	else
	{
		from.get_mut<OutputLine>()->lines.push_back(to);
	}
	if(!to.get<InputLine>())
	{
		to.set<InputLine>({{from}});
	}
	else
	{
		to.get_mut<InputLine>()->lines.push_back(from);
	}
}
