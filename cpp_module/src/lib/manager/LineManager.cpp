#include "LineManager.h"

#include <godot_cpp/variant/utility_functions.hpp>

#include <chrono>
#include <sstream>
#include <cstdlib>

#include "lib/factories/PositionedLine.h"

namespace godot {

// System categories
struct Display {};
struct Iteration {};

LineManager::~LineManager()
{
	delete _thread;
}

std::pair<flecs::entity, Position> create_line(bool horizontal, bool negative, flecs::world &ecs, std::string const &str_p, Position const &from_p, uint32_t capacity_p)
{
	Position to_l = from_p;
	int32_t diff_l = capacity_p;
	if(horizontal)
	{
		if(negative)
		{
			to_l.x -= diff_l;
		}
		else
		{
			to_l.x += diff_l;
		}
	}
	else
	{
		if(negative)
		{
			to_l.y -= diff_l;
		}
		else
		{
			to_l.y += diff_l;
		}
	}
	flecs::entity ent_l = ecs.entity(str_p.c_str())
			.set<From, Position>(from_p)
			.set<To, Position>(to_l)
			.set<Line>(Line(capacity_p));
	return std::make_pair(
		ent_l,
		to_l
	);
}

flecs::entity create_link(flecs::world &ecs, std::string const &str_p, flecs::entity &from_p, flecs::entity &to_p)
{
	Line const *line_to_l = to_p.get<Line>();
	Position const *pos_from_l = to_p.get<From, Position>();
	Position const *pos_to_l = to_p.get<To, Position>();
	int32_t unitary_x_l = (pos_to_l->x - pos_from_l->x) / int32_t(get_size(*line_to_l));
	int32_t unitary_y_l = (pos_to_l->y - pos_from_l->y) / int32_t(get_size(*line_to_l));
	Position link_from_l = *pos_from_l;
	Position link_to_l = *pos_from_l;
	link_from_l.x -= unitary_x_l;
	link_from_l.y -= unitary_y_l;
	link_to_l.x += unitary_x_l;
	link_to_l.y += unitary_y_l;
	Line line_l(2);
	flecs::entity link_l = ecs.entity(str_p.c_str())
		.set<From, Position>(link_from_l)
		.set<To, Position>(link_to_l)
		.set<Line>(line_l)
		.set<Input>({from_p.get_ref<Line>()})
		.set<Output>({to_p.get_ref<Line>()});

	from_p.set<To, Connector>({link_l});
	to_p.set<From, Connector>({link_l});

	return link_l;
}

void LineManager::init()
{
	UtilityFunctions::print("init");

	auto pair_l = create_line(true, false, ecs, "line", {3, 3}, 10);
	Position pos = pair_l.second;

	flecs::entity line = pair_l.first;
	line.add<Spawn>();

	pair_l = create_line(true, false, ecs, "line2", pos, 10);
	pos = pair_l.second;
	flecs::entity line2 = pair_l.first;
	increment_line = line2;

	create_link(ecs, "link", line, line2);

	// pair_l = create_line(false, false, ecs, "line3", pos, 10);
	// pos = pair_l.second;
	// flecs::entity line3 = pair_l.first;

	// create_link(ecs, "link2", line2, line3);

	// pair_l = create_line(true, false, ecs, "line4", pos, 2);
	// pos = pair_l.second;
	// flecs::entity line4 = pair_l.first;

	// create_link(ecs, "link3", line3, line4);

	// // splitter
	// {
	// 	pair_l = create_line(true, false, ecs, "line5", pos, 10);
	// 	flecs::entity line5 = pair_l.first;
	// 	pos.y += 1;
	// 	Position end_pos_l = pair_l.second;
	// 	pair_l = create_line(true, false, ecs, "line6", pos, 10);
	// 	flecs::entity line6 = pair_l.first;

	// 	flecs::entity splitter_l = create_link(ecs, "link5", line4, line5);
	// 	splitter_l.set<Sorter>({line6.get_ref<Line>(), line5.get_ref<Line>(), 0});
	// 	line6.set<From, Connector>({splitter_l});

	// 	// merger
	// 	{
	// 		pair_l = create_line(true, false, ecs, "line7", end_pos_l, 10);
	// 		flecs::entity line7 = pair_l.first;
	// 		increment_line = line7;
	// 		end_pos = pair_l.second;

	// 		flecs::entity merger_l = create_link(ecs, "link6", line5, line7);
	// 		merger_l.set<Merger>({line6.get_ref<Line>()});
	// 		line6.set<To, Connector>({merger_l});
	// 	}
	// }


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

	ecs.system<Line, flecs::pair<From, Position> const>()
		.kind<Iteration>()
		.with<Spawn>()
		.each([&](flecs::entity const &ent, Line &line_p, flecs::pair<From, Position> const &pos_p) {
			if(can_add(line_p) && c < 100)
			{
				++c;
				std::stringstream ss_l;
				ss_l<<"obj."<<c;
				DrawingInit drawing_l;
				drawing_l.x = pos_p->x * world_size;
				drawing_l.y = pos_p->y * world_size;
				std::uniform_int_distribution<> distrib(0, 4);
				int32_t type = distrib(_gen);
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
				flecs::entity object = ecs.entity(ss_l.str().c_str())
					.set<::Object>({type})
					.set<DrawingInit>(drawing_l);
				add_to_start(line_p, object);
			}
		});
	// Create custom pipeline
	flecs::entity iteration_pipeline = ecs.pipeline()
		.with(flecs::System)
		.with<Iteration>()
		.build();
	ecs.set_pipeline(iteration_pipeline);

	// DISPLAY (TODO)

	update_display = ecs.query<Line const, flecs::pair<From, Position>, flecs::pair<To, Position>>();
	init_display = ecs.query<DrawingInit const>();

	// INPUT

	new_line_system = ecs.system<PositionedLine const, MergeLines const>("set_up_line")
		.kind<Iteration>()
		.each([this](flecs::entity& ent, PositionedLine const &pl, MergeLines const ml) {
			ent.set<From, Position>(pl.from)
				.set<To, Position>(pl.to)
				.set<Line>(pl.line);
			increment_line = ent;

			ml.first.mut(ecs).destruct();
			ml.second.mut(ecs).destruct();

			if(pl.co_from)
			{
				replace_connectors(ent, ml.first.mut(ecs), pl.co_from.mut(ecs));
				ent.set<From, Connector>({pl.co_from});
			}
			if(pl.co_to)
			{
				replace_connectors(ent, ml.second.mut(ecs), pl.co_to.mut(ecs));
				ent.set<To, Connector>({pl.co_to});
			}

			ent.remove<PositionedLine>();
		});

	if(_drawer)
	{
		_drawer->set_time_step(time_step);
	}

	UtilityFunctions::print("done");
	_init = true;
}

void LineManager::loop()
{
	auto start{std::chrono::steady_clock::now()};

	ecs.progress();

	auto end{std::chrono::steady_clock::now()};
	std::chrono::duration<double> diff = end - start;

	// UtilityFunctions::print("total ", diff.count()*1000.);
}

void LineManager::_process(double delta)
{
	Node::_process(delta);
	if(!_init)
	{
		return;
	}
	_elapsed += delta;

	if(_elapsed >= time_step)
	{
		_elapsed -= time_step;

		// finish old loop
		if(_thread)
		{
			_thread->join();

			ecs.defer([&]{
				init_display.each([this](flecs::entity &ent, DrawingInit const &init_p) {
					Drawing drawing_l;
					drawing_l.x = init_p.x;
					drawing_l.y = init_p.y;
					FrameInfo const & sprite_frame = _framesLibrary->getFrameInfo(init_p.frame);
					drawing_l.idx = _drawer->add_instance(Vector2(drawing_l.x, drawing_l.y), sprite_frame.offset, sprite_frame.sprite_frame, "default", "", false);
					ent.set<Drawing>(drawing_l);
					ent.remove<DrawingInit>();
				});
			});
			_drawer->update_pos();

			update_display.each([this](flecs::entity const &ent, Line const &line_p, flecs::pair<From, Position> &from_p, flecs::pair<To, Position> &to_p) {

					double step_l = world_size;
					uint32_t dist = line_p.dist_end;
					size_t idx = line_p.first;
					while(idx < line_p.items.size())
					{
						ItemOnLine const &item_l = line_p.items[idx];

						Drawing * drawable_l = item_l.ent.mut(ecs).get_mut<Drawing>();
						if(drawable_l)
						{
							drawable_l->x = float(to_p->x * step_l + (from_p->x - to_p->x) * step_l * float(dist) / line_p.full_dist);
							drawable_l->y = float(to_p->y * step_l + (from_p->y - to_p->y) * step_l * float(dist) / line_p.full_dist);

							if(_drawer)
							{
								_drawer->set_new_pos(drawable_l->idx, Vector2(drawable_l->x, drawable_l->y));
							}
						}
						dist += item_l.dist_to_next+100;
						idx = item_l.next;
					}
				});

		}

		if(space_pressed)
		{
			flecs::entity ent_l = increment_line;
			Line const &line_l = *ent_l.get<Line>();
			Position const * ent_pos_l = ent_l.get_second<To, Position>();
			RefPositionedLine rpl {
				line_l,
				*ent_l.get_second<From, Position>(),
				*ent_pos_l,
				ent_l.get_second<From, Connector>()?ent_l.get_second<From, Connector>()->ent:flecs::entity_view(),
				ent_l.get_second<To, Connector>()?ent_l.get_second<To, Connector>()->ent:flecs::entity_view()
			};

			if(!ent_pos_l)
			{
				std::cout<<"error"<<std::endl;
			}
			Position pos = *ent_pos_l;
			std::stringstream ss_l;
			ss_l << "line_space" << ++offset;
			flecs::entity new_line_l = create_line(true, false, ecs, ss_l.str(), pos, 1).first;
			RefPositionedLine rpl_second {
				*new_line_l.get<Line>(),
				*new_line_l.get_second<From, Position>(),
				*new_line_l.get_second<To, Position>(),
				new_line_l.get_second<From, Connector>()?new_line_l.get_second<From, Connector>()->ent:flecs::entity_view(),
				new_line_l.get_second<To, Connector>()?new_line_l.get_second<To, Connector>()->ent:flecs::entity_view()
			};

			PositionedLine new_pl = merge_positioned_lines(rpl, rpl_second);

			// destroy items
			size_t idx = line_l.first;
			while(idx < line_l.items.size())
			{
				ItemOnLine const &item_l = line_l.items[idx];

				Drawing const * drawable_l = item_l.ent.get<Drawing>();
				if(drawable_l)
				{
					_drawer->set_animation_one_shot(drawable_l->idx, StringName("default"));
				}
				idx = item_l.next;
			}

			// create new line
			flecs::entity new_ent_l = ecs.entity()
					.set<PositionedLine>(new_pl)
					.set<MergeLines>({ent_l, new_line_l});

			space_pressed = false;
		}

		new_line_system.run();


		// new loop
		delete _thread;
		_thread = new std::thread(&LineManager::loop, this);
	}
}

void LineManager::_bind_methods()
{
	ClassDB::bind_method(D_METHOD("init"), &LineManager::init);
	ClassDB::bind_method(D_METHOD("setEntityDrawer", "drawer"), &LineManager::setEntityDrawer);
	ClassDB::bind_method(D_METHOD("getEntityDrawer"), &LineManager::getEntityDrawer);
	ClassDB::bind_method(D_METHOD("setFramesLibrary", "library"), &LineManager::setFramesLibrary);
	ClassDB::bind_method(D_METHOD("getFramesLibrary"), &LineManager::getFramesLibrary);
	ClassDB::bind_method(D_METHOD("key_pressed", "key"), &LineManager::key_pressed);

	ADD_GROUP("LineManager", "LineManager_");
}

void LineManager::setEntityDrawer(EntityDrawer *drawer_p)
{
	_drawer = drawer_p;
}
EntityDrawer *LineManager::getEntityDrawer() const
{
	return _drawer;
}

void LineManager::setFramesLibrary(FramesLibrary *lib_p)
{
	_framesLibrary = lib_p;
}

FramesLibrary *LineManager::getFramesLibrary() const
{
	return _framesLibrary;
}

void LineManager::key_pressed(int key_p)
{
	// space
	if(key_p == KEY_SPACE)
	{
		space_pressed = true;
	}
}

}
