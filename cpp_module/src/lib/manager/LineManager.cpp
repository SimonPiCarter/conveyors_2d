#include "LineManager.h"

#include <godot_cpp/variant/utility_functions.hpp>

#include <chrono>
#include <sstream>
#include <cstdlib>

#include "lib/factories/PositionedLine.h"
#include "lib/factories/FactorySystems.h"
#include "lib/pipeline/PipelineSteps.h"

namespace godot {


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

std::pair<flecs::entity, Position> create_unit_line(bool horizontal, bool negative, flecs::world &ecs, Position const &to_p)
{
	Position from_l = to_p;
	if(horizontal)
	{
		if(!negative)
		{
			from_l.x -= 1;
		}
		else
		{
			from_l.x += 1;
		}
	}
	else
	{
		if(!negative)
		{
			from_l.y -= 1;
		}
		else
		{
			from_l.y += 1;
		}
	}
	flecs::entity ent_l = ecs.entity()
			.set<From, Position>(from_l)
			.set<To, Position>(to_p)
			.set<Line>(Line(1));
	return std::make_pair(
		ent_l,
		from_l
	);
}

void add_line_display(float world_size_p, EntityDrawer &drawer_p, FramesLibrary &framesLibrary_p, flecs::entity ent)
{
	iterate_on_positions(ent, [&](Position const &pos_p) {
		Vector2 pos_instance_l { world_size_p * pos_p.x, world_size_p * pos_p.y - 1 };
		FrameInfo const & sprite_frame = framesLibrary_p.getFrameInfo("belt");
		drawer_p.add_instance(pos_instance_l, sprite_frame.offset, sprite_frame.sprite_frame, "default", "", false);
	});
}

void LineManager::init()
{
	UtilityFunctions::print("init");

	auto pair_l = create_line(true, false, ecs, "line", {3, 3}, 10);
	add_line_display(world_size, *_drawer2, *_framesLibrary, pair_l.first);
	fill(grid, pair_l.first);
	Position pos = pair_l.second;

	flecs::entity line = pair_l.first;
	line.add<Spawn>();

	pair_l = create_line(true, false, ecs, "line2", pos, 10);
	add_line_display(world_size, *_drawer2, *_framesLibrary, pair_l.first);
	fill(grid, pair_l.first);
	pos = pair_l.second;
	flecs::entity line2 = pair_l.first;
	increment_line = line2;

	create_link(ecs, "link", line, line2);

	create_factory_systems(ecs, world_size, _gen);

	// Create custom pipeline
	flecs::entity iteration_pipeline = ecs.pipeline()
		.with(flecs::System)
		.with<Iteration>()
		.build();
	ecs.set_pipeline(iteration_pipeline);

	// DISPLAY (TODO)

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

		// only dequeue one by one
		if(!_line_spawn_queue.empty())
		{
			SpawnLine line_l = _line_spawn_queue.front();

			flecs::entity new_line_l = create_unit_line(line_l.horizontal, line_l.negative, ecs, {line_l.x, line_l.y}).first;
			if(check_line(grid, new_line_l))
			{
				add_line_display(world_size, *_drawer2, *_framesLibrary, new_line_l);
				fill(grid, new_line_l);

				merge_around(_drawer, grid, ecs, new_line_l);
			}
			else
			{
				new_line_l.destruct();
			}

			_line_spawn_queue.pop_front();
		}

		if(space_pressed)
		{
			flecs::entity ent_l = increment_line;
			Position const * ent_pos_l = ent_l.get_second<To, Position>();

			// Added new line
			Position pos = *ent_pos_l;
			std::stringstream ss_l;
			ss_l << "line_space" << ++offset;
			flecs::entity new_line_l = create_line(true, false, ecs, ss_l.str(), pos, 1).first;
			add_line_display(world_size, *_drawer, *_framesLibrary, new_line_l);
			fill(grid, new_line_l);

			// create new merged line
			flecs::entity new_ent_l = merge_around(_drawer, grid, ecs, new_line_l);
			increment_line = new_ent_l;

			space_pressed = false;
		}

		// merge_line_system.run();


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
	ClassDB::bind_method(D_METHOD("setEntityDrawer2", "drawer"), &LineManager::setEntityDrawer2);
	ClassDB::bind_method(D_METHOD("getEntityDrawer2"), &LineManager::getEntityDrawer2);
	ClassDB::bind_method(D_METHOD("setFramesLibrary", "library"), &LineManager::setFramesLibrary);
	ClassDB::bind_method(D_METHOD("getFramesLibrary"), &LineManager::getFramesLibrary);
	ClassDB::bind_method(D_METHOD("get_world_size"), &LineManager::get_world_size);
	ClassDB::bind_method(D_METHOD("spawn_line", "x", "y", "horizontal", "negative"), &LineManager::spawn_line);

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

void LineManager::setEntityDrawer2(EntityDrawer *drawer_p)
{
	_drawer2 = drawer_p;
}
EntityDrawer *LineManager::getEntityDrawer2() const
{
	return _drawer2;
}

void LineManager::setFramesLibrary(FramesLibrary *lib_p)
{
	_framesLibrary = lib_p;
}

FramesLibrary *LineManager::getFramesLibrary() const
{
	return _framesLibrary;
}

void LineManager::spawn_line(int x, int y, bool honrizontal_p, bool negative_p)
{
	_line_spawn_queue.push_back({(int32_t)x, (int32_t)y, honrizontal_p, negative_p});
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
