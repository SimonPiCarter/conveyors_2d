#include "DrawingSystems.h"

#include "entity_drawer/EntityDrawer.h"
#include "entity_drawer/FramesLibrary.h"

#include "lib/drawing/Drawing.h"
#include "lib/pipeline/PipelineSteps.h"

#include "lib/manager/LineManager.h"

using namespace godot;

void set_up_display_systems(flecs::world &ecs, godot::LineManager *manager_p) {

	ecs.system<DrawingInit const>()
		.kind<Display>()
		.each([manager_p](flecs::entity &ent, DrawingInit const &init_p) {
			Drawing drawing_l;
			init_p.x;
			init_p.y;
			FrameInfo const & sprite_frame = manager_p->getFramesLibrary()->getFrameInfo(init_p.frame);
			drawing_l.idx = manager_p->getEntityDrawer()->add_instance(
				Vector2(init_p.x, init_p.y),
				sprite_frame.offset,
				sprite_frame.sprite_frame,
				"default",
				"",
				false);
			ent.set<Drawing>(drawing_l);
			ent.remove<DrawingInit>();
		});

	ecs.system("update_pos")
		.kind<Display>()
		.iter([manager_p](flecs::iter& it) {
			manager_p->getEntityDrawer()->update_pos();
			manager_p->getEntityDrawer2()->update_pos();
		});

	ecs.system<Drawing const>()
		.kind<Display>()
		.with<Consumed>()
		.each([manager_p](flecs::entity const &ent, Drawing const &drawing_p) {
			manager_p->getEntityDrawer()->set_animation_one_shot(drawing_p.idx, "default");
			ent.destruct();
		});

	ecs.system<Line const, CellLine const>()
		.kind<Display>()
		.each([&ecs, manager_p](flecs::entity const &ent, Line const &line_p, CellLine const &cell_line_p) {
			double step_l = manager_p->get_world_size();
			uint32_t dist = line_p.dist_end;
			size_t idx = line_p.first;
			LinePosition from_l = cell_line_p.start;
			LinePosition to_l = cell_line_p.end;
			while(idx < line_p.items.size())
			{
				ItemOnLine const &item_l = line_p.items[idx];

				Drawing const * drawable_l = item_l.ent.get<Drawing>();
				if(drawable_l)
				{
					float x = float(to_l.x * step_l + (from_l.x - to_l.x) * step_l * float(dist) / line_p.full_dist);
					float y = float(to_l.y * step_l + (from_l.y - to_l.y) * step_l * float(dist) / line_p.full_dist);

					if(manager_p->getEntityDrawer())
					{
						manager_p->getEntityDrawer()->set_new_pos(drawable_l->idx, Vector2(x, y));
					}
				}
				dist += item_l.dist_to_next+100;
				idx = item_l.next;
			}
		});
}
