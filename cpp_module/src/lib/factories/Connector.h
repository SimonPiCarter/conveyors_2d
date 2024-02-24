#pragma once

#include "flecs.h"
#include "Line.h"

struct Connector {
	flecs::entity_view ent;
};

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

struct Sorter {
	flecs::ref<Line> out_type;
	flecs::ref<Line> out_non_type;
	int32_t type;
};

void replace_connectors(flecs::entity new_p, flecs::entity_view old_p, flecs::entity connector_p);
