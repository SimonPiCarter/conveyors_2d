#pragma once

#include "flecs.h"
#include <string>
#include <vector>

namespace godot
{
	class EntityDrawer;
}

struct Drawing {
	int idx = 0;
};

struct DrawingInit {
	float x = 0;
	float y = 0;
	std::string frame;
};

struct DrawingLine {
	std::vector<int> indexes;
};

void clean_up_line(godot::EntityDrawer * drawer_p, flecs::entity_view ent_p);
void remove_display_line(godot::EntityDrawer * drawer_p, flecs::entity_view ent_p);
