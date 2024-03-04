#include "Merger.h"

#include "lib/pipeline/PipelineSteps.h"

void add_merger_system(flecs::world &ecs)
{
	ecs.system<Merger>()
		.kind<Iteration>()
		.each([](flecs::entity ent, Merger &s) {
			Line * line_in_cur = s.in[s.cur].try_get();

			// if input line has sent an object to its output
			if(!line_in_cur || line_in_cur->sent_to_next) {
				// we switch current between 0 and 1
				s.cur = 1 - s.cur;
			}

			line_in_cur = s.in[s.cur].try_get();
			Line * line_in_alt = s.in[1 - s.cur].try_get();

			// if no line cur or if alt has an incoming object
			if((!line_in_cur || line_in_cur->dist_end > line_in_cur->speed)
			 &&(line_in_alt && line_in_alt->dist_end <= line_in_alt->speed)){
				// we switch current between 0 and 1
				s.cur = 1 - s.cur;
			}

			line_in_cur = s.in[s.cur].try_get();
			line_in_alt = s.in[1 - s.cur].try_get();

			// update line in next line
			line_in_cur->next_line = s.out.try_get();
			line_in_alt->next_line = nullptr;
		});
}
