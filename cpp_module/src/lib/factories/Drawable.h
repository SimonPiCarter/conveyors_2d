#pragma once

#include "flecs.h"
#include <string>

namespace godot
{
	class EntityDrawer;
}

struct Drawing {
	int idx = 0;
	float x = 0;
	float y = 0;
};

struct DrawingInit {
	float x = 0;
	float y = 0;
	std::string frame;
};

void clean_up_line(godot::EntityDrawer * drawer_p, flecs::entity_view ent_p);
