#include "FactorySystems.h"

#include "Connector.h"
#include "Drawable.h"
#include "Line.h"
#include "Storer.h"
#include "lib/pipeline/PipelineSteps.h"

void create_factory_systems(flecs::world &ecs, float const &world_size_p, std::mt19937 &gen_p)
{
	ecs.system<Line const, Merger, Input>()
		.kind<Iteration>()
		.each([](flecs::entity const &ent, Line const &line_p, Merger &merger_p, Input &in_p) {
			Line *other_p = merger_p.prev.try_get();
			// swap iif merger has an alternative and it can consume
			if(other_p && can_consume(*other_p) && can_add(line_p))
			{
				std::swap(merger_p.prev, in_p.prev);
			}
		});

	ecs.system<Line, Input>()
		.kind<Iteration>()
		.each([](flecs::entity const &ent, Line &line_p, Input &in_p) {
			Line *prev_p = in_p.prev.try_get();
			if(prev_p)
			{
				if(can_consume(*prev_p) && can_add(line_p))
				{
					add_to_start(line_p, consume(*prev_p));
				}
			}
		});

	ecs.system<Line>()
		.kind<Iteration>()
		.each([](flecs::entity const &ent, Line &line_p) {
			step(line_p);
		});

	ecs.system<Line const, Splitter, Output>()
		.kind<Iteration>()
		.each([](flecs::entity const &ent, Line const &line_p, Splitter &splitter_p, Output &out_p) {
			Line *other_p = splitter_p.next.try_get();
			// swap iif splitter has an alternative and it can add
			if(other_p && can_add(*other_p) && can_consume(line_p))
			{
				std::swap(splitter_p.next, out_p.next);
			}
		});

	ecs.system<Line const, Sorter const, Output>()
		.kind<Iteration>()
		.each([](flecs::entity const &ent, Line const &line_p, Sorter const &sorter_p, Output &out_p) {
			flecs::entity_view consumable_l = can_consume(line_p);
			if(consumable_l)
			{
				if(consumable_l.get<::Object>()->type == sorter_p.type)
				{
					out_p.next = sorter_p.out_type;
				}
				else
				{
					out_p.next = sorter_p.out_non_type;
				}
			}
		});

	ecs.system<Line, Output>()
		.kind<Iteration>()
		.each([](flecs::entity const &ent, Line &line_p, Output &out_p) {
			Line *next_p = out_p.next.try_get();
			if(next_p)
			{
				if(can_consume(line_p) && can_add(*next_p))
				{
					add_to_start(*next_p, consume(line_p));
				}
			}
		});

	ecs.system<Line, flecs::pair<From, Position> const, Spawn const>()
		.kind<Iteration>()
		.each([&](flecs::entity const &ent, Line &line_p, flecs::pair<From, Position> const &pos_p, Spawn const &spawn_p) {
			if(can_add(line_p) && !spawn_p.types.empty())
			{
				DrawingInit drawing_l;
				drawing_l.x = pos_p->x * world_size_p;
				drawing_l.y = pos_p->y * world_size_p;
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
					.set<::Object>({type})
					.set<DrawingInit>(drawing_l);
				add_to_start(line_p, object);
			}
		});

	ecs.system<Line, ConnectedToStorer const>()
		.kind<Iteration>()
		.each([&](flecs::entity const &ent, Line &line_p, ConnectedToStorer const &storer_p) {
			if(can_consume(line_p))
			{
				flecs::entity_view ent_consumed_l = consume(line_p);
				Object const *object_l = ent_consumed_l.get<Object>();
				if(object_l && storer_p.storer_ent.get<Storer>())
				{
					Storer * storer_l = storer_p.storer_ent.mut(ent).get_mut<Storer>();
					add_to_storage(*storer_l, object_l->type);
					ent_consumed_l.mut(ent).add<Consumed>();
				}
			}
		});
}
