#pragma once

#include "flecs.h"

namespace godot
{
	class LineManager;
} // namespace godot

void set_up_display_systems(flecs::world &ecs, godot::LineManager *manager_p);
