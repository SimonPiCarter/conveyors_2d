#include "Drawing.h"

#include <godot_cpp/variant/utility_functions.hpp>
#include "entity_drawer/EntityDrawer.h"
#include "lib/line/Line.h"

void clean_up_line(godot::EntityDrawer * drawer_p, flecs::entity_view ent_p)
{
	// destroy items
	Line const &line_l = *ent_p.get<Line>();
	size_t idx = line_l.first;
	while(idx < line_l.items.size())
	{
		ItemOnLine const &item_l = line_l.items[idx];

		Drawing const * drawable_l = item_l.ent.get<Drawing>();
		if(drawable_l)
		{
			drawer_p->set_animation_one_shot(drawable_l->idx, godot::StringName("default"));
		}
		idx = item_l.next;
	}
}

void remove_display_line(godot::EntityDrawer * drawer_p, flecs::entity_view ent_p)
{
	DrawingLine const *d_line_l = ent_p.get<DrawingLine>();
	if(d_line_l)
	{
		for(int idx_l : d_line_l->indexes)
		{
			drawer_p->set_animation_one_shot(idx_l, godot::StringName("default"));
		}
	}
}
