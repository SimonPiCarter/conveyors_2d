#pragma once

#include "flecs.h"

#include <vector>
#include <list>

struct From {};
struct To {};

struct Position {
	int32_t x, y;
};

struct Object {};

struct ItemOnLine
{
	flecs::entity_view ent;
	/// @brief index to next element in the list
	size_t next = 0;
	int32_t dist_to_next = 0;
};

struct Line {
	Line() = default;
	Line(uint32_t capacity_p);

	std::vector<ItemOnLine> items;
	/// @brief list of free idx in the vector
	std::list<size_t> free_idx;
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

struct Spawn {};

struct Input {
	flecs::ref<Line> prev;
};

struct Output {
	flecs::ref<Line> next;
};

struct Splitter {
	flecs::ref<Line> next;
};

struct Merger {
	flecs::ref<Line> prev;
};

/// @brief performs a step on a given line
void step(Line &line_p);

bool can_add(Line const &line_p);
flecs::entity_view can_consume(Line const &line_p);
bool add_to_start(Line &line_p, flecs::entity_view const &item_p);
flecs::entity_view consume(Line &line_p);

size_t get_content_size(Line const &line_p);
size_t get_size(Line const &line_p);

bool is_empty(Line const &line_p);
