#include "LineManager.h"

#include <godot_cpp/variant/utility_functions.hpp>

#include <chrono>
#include <sstream>

namespace godot {

LineManager::~LineManager()
{
	delete _thread;
}

void LineManager::init()
{
	UtilityFunctions::print("init");

	flecs::entity line = ecs.entity("line")
		.set<From, Position>({10, 10})
		.set<To, Position>({50, 10})
		.set<Line>(Line(10))
		.add<Spawn>();

	flecs::entity line2 = ecs.entity("line2")
		.set<From, Position>({50, 10})
		.set<To, Position>({50, 50})
		.set<Line>(Line(10));

	flecs::entity line3 = ecs.entity("line3")
		.set<From, Position>({50, 50})
		.set<To, Position>({58, 50})
		.set<Line>(Line(2));

	flecs::entity line4 = ecs.entity("line4")
		.set<From, Position>({58, 50})
		.set<To, Position>({58, 10})
		.set<Line>(Line(10));

	flecs::entity line5 = ecs.entity("line5")
		.set<From, Position>({58, 10})
		.set<To, Position>({66, 10})
		.set<Line>(Line(2));

	flecs::entity line6 = ecs.entity("line6")
		.set<From, Position>({66, 10})
		.set<To, Position>({66, 50})
		.set<Line>(Line(10));

	ecs.entity()
		.set<From, Position>({50, 10})
		.set<To, Position>({50, 10})
		.set<Line>(Line(1))
		.set<Link>({line.get_ref<Line>(), line2.get_ref<Line>()});

	ecs.entity()
		.set<From, Position>({50, 50})
		.set<To, Position>({50, 50})
		.set<Line>(Line(1))
		.set<Link>({line2.get_ref<Line>(), line3.get_ref<Line>()});

	ecs.entity()
		.set<From, Position>({58, 50})
		.set<To, Position>({58, 50})
		.set<Line>(Line(1))
		.set<Link>({line3.get_ref<Line>(), line4.get_ref<Line>()});

	ecs.entity()
		.set<From, Position>({58, 10})
		.set<To, Position>({58, 10})
		.set<Line>(Line(1))
		.set<Link>({line4.get_ref<Line>(), line5.get_ref<Line>()});

	ecs.entity()
		.set<From, Position>({66, 10})
		.set<To, Position>({66, 10})
		.set<Line>(Line(1))
		.set<Link>({line5.get_ref<Line>(), line6.get_ref<Line>()});

	ecs.system<Line, Link>()
		.each([](flecs::entity const &ent, Line &line_p, Link &link_p) {
			Line *prev_p = link_p.prev.try_get();
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

	ecs.system<Line, Link>()
		.each([](flecs::entity const &ent, Line &line_p, Link &link_p) {
			Line *next_p = link_p.next.try_get();
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
			if(can_add(line_p) && c < 10)
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
