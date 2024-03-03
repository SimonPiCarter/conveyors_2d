#include <iostream>

#include "Line.h"
#include "grid/Grid.h"

int main()
{
	flecs::world ecs;

	flecs::entity e1 = ecs.entity("e1")
		.add<Spawn>()
		.set<Line>(Line(1));
	flecs::entity e2 = ecs.entity("e2")
		.set<Line>(Line(1));
	flecs::entity e3 = ecs.entity("e3")
		.set<Line>(Line(1));


	flecs::entity e4 = ecs.entity("e4")
		.add<Spawn>()
		.set<Line>(Line(1));
	flecs::entity e5 = ecs.entity("e5")
		.set<Line>(Line(1));

	connect(e1, e2);
	connect(e2, e3);
	connect(e3, e5);
	connect(e4, e5);

	flecs::entity obj1 = ecs.entity("obj1");
	flecs::entity obj2 = ecs.entity("obj2");
	neo_add_to_start(*e1.get_mut<Line>(), obj1, 0);
	neo_add_to_start(*e2.get_mut<Line>(), obj2, 0);

	//
	// e1 - e2 - e3 - e5
	// e4 ------------/
	//

	tag_all_magnitude(ecs);

	ecs.system<const OutputLine, Line>()
		.each([](OutputLine const &ol, Line &line_p) {
			line_p.next_line = ol.lines[ol.current].get_mut<Line>();
		});

	// Use readonly term for component used for sorting
	ecs.system<const Magnitude, Line>()
		.order_by<Magnitude>([](flecs::entity_t e1, const Magnitude *d1, flecs::entity_t e2, const Magnitude *d2) {
			return (d1->order < d2->order) - (d1->order > d2->order);
		})
		.each([](flecs::entity ent, const Magnitude &m, Line &line_p) {
			std::cout<< ent.name() <<" order = " << m.order << std::endl;

			for_each_item(line_p, [](ItemOnLine const &item, int32_t pos) {
				std::cout<<item.ent.name()<<" pos : "<<pos<<std::endl;
			});
			neo_step(line_p);
		});


	ecs.progress();
	std::cout<<std::endl;
	ecs.progress();

	Grid grid_l(256, 256);

	grid_l.set(10, 8, create_up(ecs, 10, 8));
	grid_l.set(10, 10, create_up(ecs, 10, 10));
	grid_l.set(10, 9, create_up(ecs, 10, 9));
	grid_l.set(13, 11, create_left(ecs, 13, 11));
	grid_l.set(11, 11, create_left(ecs, 11, 11));
	grid_l.set(10, 11, create_left_up(ecs, 10, 11));
	grid_l.set(12, 11, create_left(ecs, 12, 11));

	merge_all_cells(ecs, grid_l);

	ecs.filter<CellLine const>()
		.each([&](CellLine const &line_p) {
			std::cout<<line_p<<std::endl;
		});

	return 0;
}
