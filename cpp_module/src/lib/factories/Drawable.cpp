#include "Drawable.h"

#include "entity_drawer/EntityDrawer.h"
#include "Line.h"

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
