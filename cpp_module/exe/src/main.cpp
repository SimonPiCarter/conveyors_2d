#include <iostream>

#include "Line.h"
#include "grid/Grid.h"

int main()
{
	flecs::world ecs;

	ecs.system<const OutputLine, Line>()
		.each([](OutputLine const &ol, Line &line_p) {
			line_p.next_line = ol.lines[ol.current].get_mut<Line>();
		});


	ecs.system<const Magnitude, Line const>()
		.order_by<Magnitude>([](flecs::entity_t e1, const Magnitude *d1, flecs::entity_t e2, const Magnitude *d2) {
			return (d1->order < d2->order) - (d1->order > d2->order);
		})
		.each([](flecs::entity ent, const Magnitude &m, Line const &line_p) {
			std::cout<< ent.name() <<" order = " << m.order << std::endl;

			for_each_item(line_p, [](ItemOnLine const &item, int32_t pos) {
				std::cout<<item.ent.name()<<" pos : "<<pos<<std::endl;
			});
		});

	// Use readonly term for component used for sorting
	ecs.system<const Magnitude, Line>()
		.order_by<Magnitude>([](flecs::entity_t e1, const Magnitude *d1, flecs::entity_t e2, const Magnitude *d2) {
			return (d1->order < d2->order) - (d1->order > d2->order);
		})
		.each([](flecs::entity ent, const Magnitude &m, Line &line_p) {
			neo_step(line_p);
		});

	Grid grid_l(256, 256);

	grid_l.set(10, 8, create_up(ecs, 10, 8));
	grid_l.set(10, 10, create_up(ecs, 10, 10));
	grid_l.set(10, 9, create_up(ecs, 10, 9));

	grid_l.set(13, 11, create_left(ecs, 13, 11));
	grid_l.set(11, 11, create_left(ecs, 11, 11));
	grid_l.set(12, 11, create_left(ecs, 12, 11));

	grid_l.set(14, 10, create_down(ecs, 14, 10));
	grid_l.set(14, 9, create_down(ecs, 14, 9));
	grid_l.set(14, 8, create_down(ecs, 14, 8));

	flecs::entity ent = create_empty(ecs, 10, 11);
	create_cell_half_line_right(ecs, ent, true);
	create_cell_half_line_up(ecs, ent, false);

	ent = create_empty(ecs, 14, 11);
	create_cell_half_line_left(ecs, ent, false);
	create_cell_half_line_up(ecs, ent, true);

	merge_all_cells(ecs, grid_l);
	create_all_lines(ecs);
	link_all_simple_cells(ecs);

	flecs::entity obj1 = ecs.entity("obj1");
	flecs::entity cell = grid_l.get(14, 8);
	Cell const * cell_component = cell.get<Cell>();
	neo_add_to_start(*cell_component->lines[0].get_mut<Line>(), obj1, 0);

	ecs.filter<CellLine const>()
		.each([&](CellLine const &line_p) {
			std::cerr<<line_p<<std::endl;
		});

	tag_all_magnitude(ecs);

	std::cerr<<"progress"<<std::endl;
	ecs.progress();
	for(size_t i = 1 ; i < 20 ; ++ i)
	{
		std::cout<<"i = "<<i<<std::endl;
		ecs.progress();
	}

	return 0;
}
