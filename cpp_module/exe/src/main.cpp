#include <iostream>

#include "lib/line/Line.h"
#include "lib/line/systems/LineSystems.h"
#include "lib/grid/Grid.h"
#include "lib/pipeline/PipelineSteps.h"

int main()
{
	flecs::world ecs;

	uint32_t timestamp_l = 0;
	std::mt19937 rand_l(42);

	set_up_line_systems(ecs, timestamp_l, timestamp_l, rand_l, true);

	Grid grid_l(256, 256);

	grid_l.set(10, 8, create_up(ecs, 10, 8));
	grid_l.set(10, 10, create_up(ecs, 10, 10));
	grid_l.set(10, 9, create_up(ecs, 10, 9));
	grid_l.set(10, 7, create_up(ecs, 10, 7));
	grid_l.set(10, 6, create_up(ecs, 10, 6));

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

	flecs::entity cell = grid_l.get(14, 8);
	Cell const * cell_component = cell.get<Cell>();
	flecs::entity cell_line = cell_component->lines[0];
	cell_line.set<Spawn>({{0}, 0, 0});

	ecs.filter<CellLine const>()
		.each([&](CellLine const &line_p) {
			std::cerr<<line_p<<std::endl;
		});

	tag_all_magnitude(ecs);

	// Create custom pipeline
	flecs::entity iteration_pipeline = ecs.pipeline()
		.with(flecs::System)
		.with<Iteration>()
		.build();
	ecs.set_pipeline(iteration_pipeline);

	std::cerr<<"progress"<<std::endl;
	for(size_t i = 0 ; i < 20 ; ++ i)
	{
		++timestamp_l;
		std::cout<<"i = "<<i<<std::endl;
		ecs.progress();
	}

	return 0;
}
