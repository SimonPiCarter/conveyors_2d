#pragma once

#include <list>
#include <functional>
#include <map>
#include <set>
#include <vector>

#include "flecs.h"

struct Spawn {};

struct ItemOnLine {
	flecs::entity_view ent;
	/// @brief index to next element in the list
	size_t next = 0;
	int32_t dist_to_next = 0;
};

struct Line {
	Line() = default;
	Line(uint32_t distance_p) : first(size_t(distance_p/100)), dist_start(distance_p), dist_end(distance_p), full_dist(distance_p)
	{
		items.resize(distance_p/100);
		for(size_t i = 0 ; i < items.size(); ++i)
		{
			free_idx.push_back(i);
		}
	}

	std::vector<ItemOnLine> items;
	/// @brief list of free idx in the vector
	std::list<size_t> free_idx;

	// pointer the the next line
	Line *next_line = nullptr;

	/// @brief index of first item in the list
	size_t first = 0;
	/// @brief index of last item in the list
	size_t last = 0;

	// distances
	uint32_t dist_start = 0;
	uint32_t dist_end = 0;

	// globals
	uint32_t full_dist = 0;
	uint32_t speed = 50;
};

bool is_empty(Line const &line_p);

bool can_add(Line &line_p);

bool neo_add_to_start(Line &line_p, flecs::entity_view const &item_p, uint32_t movement_p);

int32_t get_max_movement(Line const &line_p, size_t last_p, size_t cur_p);

flecs::entity_view remove_first_from_line(Line &line_p);

void neo_step(Line &line_p);

void for_each_item(Line const &line_p, std::function<void(ItemOnLine const &, int32_t)> const &func_p);

struct Magnitude {
	uint32_t order = 0;
};

struct InputLine {
	std::vector<flecs::entity> lines;
	size_t current = 0;
};

struct OutputLine {
	std::vector<flecs::entity> lines;
	size_t current = 0;
};

template <typename T>
void update_line(T &component) {
	++component.current;
	if(component.current == component.lines.size()) {
		component.current = 0;
	}
}

void tag_magnitude(flecs::entity ent, std::vector<flecs::entity> &nexts_out);

void tag_all_magnitude(flecs::world &ecs);

void connect(flecs::entity from, flecs::entity to);
