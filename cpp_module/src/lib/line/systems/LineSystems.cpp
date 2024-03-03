#include "LineSystems.h"

#include <godot_cpp/variant/utility_functions.hpp>
#include <iostream>

#include "lib/pipeline/PipelineSteps.h"
#include "lib/grid/Cell.h"
#include "lib/drawing/Drawing.h"

void set_up_line_systems(flecs::world &ecs, uint32_t const &timestamp_p, float const &world_size_p, std::mt19937 &gen_p, bool print_p) {
	ecs.system<Line, CellLine const, Spawn>()
		.kind<Iteration>()
		.each([&](flecs::entity const &ent, Line &line_p, CellLine const &cell_line_p, Spawn &spawn_p) {
			if(can_add(line_p)
			&& (timestamp_p >= spawn_p.spawn_cooldown + spawn_p.last_spawn_timestamp || spawn_p.last_spawn_timestamp == 0)
			&& !spawn_p.types.empty())
			{
				LinePosition pos_l = cell_line_p.start;
				spawn_p.last_spawn_timestamp = timestamp_p + 1;
				DrawingInit drawing_l;
				drawing_l.x = pos_l.x * world_size_p;
				drawing_l.y = pos_l.y * world_size_p;
				std::uniform_int_distribution<> distrib(0, spawn_p.types.size()-1);
				int32_t type = spawn_p.types[distrib(gen_p)];
				switch(type)
				{
				case 0:
					drawing_l.frame = "blue";
					break;
				case 1:
					drawing_l.frame = "red";
					break;
				case 2:
					drawing_l.frame = "pink";
					break;
				case 3:
					drawing_l.frame = "yellow";
					break;
				default:
					drawing_l.frame = "green";
					break;
				}
				flecs::entity object = ecs.entity()
					.set<Object>({type})
					.set<DrawingInit>(drawing_l);
				neo_add_to_start(line_p, object, 0);
			}
		});

	ecs.system<const OutputLine, Line>()
		.kind<Iteration>()
		.each([](OutputLine const &ol, Line &line_p) {
			line_p.next_line = ol.lines[ol.current].get_mut<Line>();
		});


	if(print_p)
	{
		ecs.system<const Magnitude, Line const>()
			.kind<Iteration>()
			.order_by<Magnitude>([](flecs::entity_t e1, const Magnitude *d1, flecs::entity_t e2, const Magnitude *d2) {
				return (d1->order < d2->order) - (d1->order > d2->order);
			})
			.each([](flecs::entity ent, const Magnitude &m, Line const &line_p) {
				std::cout<< ent.name() <<" order = " << m.order << std::endl;

				for_each_item(line_p, [](ItemOnLine const &item, int32_t pos) {
					std::cout<<item.ent.name()<<" pos : "<<pos<<std::endl;
				});
			});
	}

	// Use readonly term for component used for sorting
	ecs.system<const Magnitude, Line>()
		.kind<Iteration>()
		.order_by<Magnitude>([](flecs::entity_t e1, const Magnitude *d1, flecs::entity_t e2, const Magnitude *d2) {
			return (d1->order < d2->order) - (d1->order > d2->order);
		})
		.each([](flecs::entity ent, const Magnitude &m, Line &line_p) {
			neo_step(line_p);
		});
}
