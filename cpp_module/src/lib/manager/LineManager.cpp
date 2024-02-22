#include "LineManager.h"

#include <godot_cpp/variant/utility_functions.hpp>

#include <chrono>
#include <sstream>

namespace godot {

LineManager::~LineManager()
{
	delete _thread;
}

std::pair<flecs::entity, Position> create_line(bool horizontal, bool negative, flecs::world &ecs, std::string const &str_p, Position const &from_p, uint32_t capacity_p)
{
	Position to_l = from_p;
	int32_t diff_l = capacity_p * 4;
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

	return link_l;
}

void LineManager::init()
{
	UtilityFunctions::print("init");

	auto pair_l = create_line(true, false, ecs, "line", {10, 10}, 10);
	Position pos = pair_l.second;

	flecs::entity line = pair_l.first;
	line.add<Spawn>();

	pair_l = create_line(true, false, ecs, "line2", pos, 10);
	pos = pair_l.second;
	flecs::entity line2 = pair_l.first;

	create_link(ecs, "link", line, line2);

	pair_l = create_line(false, false, ecs, "line3", pos, 10);
	pos = pair_l.second;
	flecs::entity line3 = pair_l.first;

	create_link(ecs, "link2", line2, line3);

	pair_l = create_line(true, false, ecs, "line4", pos, 2);
	pos = pair_l.second;
	flecs::entity line4 = pair_l.first;

	create_link(ecs, "link3", line3, line4);

	// splitter
	{
		pair_l = create_line(true, false, ecs, "line5", pos, 10);
		flecs::entity line5 = pair_l.first;
		pos.y += world_size;
		Position end_pos_l = pair_l.second;
		pair_l = create_line(true, false, ecs, "line6", pos, 10);
		flecs::entity line6 = pair_l.first;

		flecs::entity splitter_l = create_link(ecs, "link5", line4, line5);
		splitter_l.set<Splitter>({line6.get_ref<Line>()});

		// merger
		{
			pair_l = create_line(true, false, ecs, "line7", end_pos_l, 10);
			flecs::entity line7 = pair_l.first;

			flecs::entity splitter_l = create_link(ecs, "link6", line5, line7);
			splitter_l.set<Merger>({line6.get_ref<Line>()});
		}
	}


	ecs.system<Line const, Merger, Input>()
		.each([](flecs::entity const &ent, Line const &line_p, Merger &merger_p, Input &in_p) {
			Line *other_p = merger_p.prev.try_get();
			// swap iif merger has an alternative and it can consume
			if(other_p && can_consume(*other_p) && can_add(line_p))
			{
				std::swap(merger_p.prev, in_p.prev);
			}
		});

	ecs.system<Line, Input>()
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
		.each([](flecs::entity const &ent, Line &line_p) {
			step(line_p);
		});

	ecs.system<Line const, Splitter, Output>()
		.each([](flecs::entity const &ent, Line const &line_p, Splitter &splitter_p, Output &out_p) {
			Line *other_p = splitter_p.next.try_get();
			// swap iif splitter has an alternative and it can add
			if(other_p && can_add(*other_p) && can_consume(line_p))
			{
				std::swap(splitter_p.next, out_p.next);
			}
		});

	ecs.system<Line, Output>()
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
				drawing_l.frame = "blue";
				flecs::entity object = ecs.entity(ss_l.str().c_str())
					.add<::Object>()
					.set<DrawingInit>(drawing_l);
				add_to_start(line_p, object);
			}
		});

	update_display = ecs.query<Line const, flecs::pair<From, Position>, flecs::pair<To, Position>>();
	init_display = ecs.query<DrawingInit const>();

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

			init_display.each([this](flecs::entity &ent, DrawingInit const &init_p) {
				Drawing drawing_l;
				drawing_l.x = init_p.x;
				drawing_l.y = init_p.y;
				FrameInfo const & sprite_frame = _framesLibrary->getFrameInfo(init_p.frame);
				drawing_l.idx = _drawer->add_instance(Vector2(drawing_l.x, drawing_l.y), sprite_frame.offset, sprite_frame.sprite_frame, "default", "", false);
				ent.set<Drawing>(drawing_l);
				ent.remove<DrawingInit>();
			});
			_drawer->update_pos();

			update_display.each([this](flecs::entity const &ent, Line const &line_p, flecs::pair<From, Position> &from_p, flecs::pair<To, Position> &to_p) {

					float step_l = world_size;
					uint32_t dist = line_p.dist_end;
					size_t idx = line_p.first;
					while(idx < line_p.items.size())
					{
						ItemOnLine const &item_l = line_p.items[idx];

						Drawing * drawable_l = item_l.ent.mut(ecs).get_mut<Drawing>();
						if(drawable_l)
						{
							drawable_l->x = to_p->x * step_l + (from_p->x - to_p->x) * step_l * float(dist) / line_p.full_dist;
							drawable_l->y = to_p->y * step_l + (from_p->y - to_p->y) * step_l * float(dist) / line_p.full_dist;

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


}
