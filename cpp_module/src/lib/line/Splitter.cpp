#include "Splitter.h"

#include "lib/pipeline/PipelineSteps.h"

void add_splitter_system(flecs::world &ecs)
{
	ecs.system<Splitter>()
		.kind<Iteration>()
		.each([](flecs::entity ent, Splitter &s) {
			Line * line_in = s.in.try_get();

			// if input line has sent an object to its output
			if(line_in->sent_to_next) {
				// we switch current between 0 and 1
				s.cur = 1 - s.cur;
			}

			// if current line out has no room we switch
			Line * line_out_cur = s.out[s.cur].try_get();
			if(!line_out_cur || !can_add(*line_out_cur)) {
				// we switch current between 0 and 1
				s.cur = 1 - s.cur;
			}

			// update line in next line
			line_in->next_line = s.out[s.cur].try_get();
		});
}
