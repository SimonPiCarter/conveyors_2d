#include "LineManager.h"

#include <godot_cpp/variant/utility_functions.hpp>

#include <chrono>
#include <sstream>
#include <cstdlib>

#include "lib/factories/Storer.h"
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
	line.set<Spawn>({{2}});

	pair_l = create_line(true, false, ecs, "line2", pos, 10);
	add_line_display(world_size, *_drawer2, *_framesLibrary, pair_l.first);
	fill(grid, pair_l.first);
	pos = pair_l.second;
	flecs::entity line2 = pair_l.first;

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
	consumed_objects = ecs.query<Drawing const, Consumed const>();

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

			consumed_objects.each([this](flecs::entity const &ent, Drawing const &drawing_p, Consumed const) {
				_drawer->set_animation_one_shot(drawing_p.idx, "default");
				ent.destruct();
			});

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
		while(!_line_spawn_queue.empty())
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

		while(!_splitter_spawn_queue.empty())
		{
			SpawnSplitter splitter_l = _splitter_spawn_queue.front();

			spawn_splitter_internal(splitter_l.x, splitter_l.y, splitter_l.horizontal, splitter_l.negative, splitter_l.flipped);
			_splitter_spawn_queue.pop_front();
		}

		while(!_merger_spawn_queue.empty())
		{
			SpawnMerger splitter_l = _merger_spawn_queue.front();

			spawn_merger_internal(splitter_l.x, splitter_l.y, splitter_l.horizontal, splitter_l.negative, splitter_l.flipped);
			_merger_spawn_queue.pop_front();
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

void LineManager::spawn_splitter(int x, int y, bool horizontal_p, bool negative_p, bool flipped_p)
{
	_splitter_spawn_queue.push_back({(int32_t)x, (int32_t)y, horizontal_p, negative_p, false});
}

void LineManager::spawn_merger(int x, int y, bool horizontal_p, bool negative_p, bool flipped_p)
{
	_merger_spawn_queue.push_back({(int32_t)x, (int32_t)y, horizontal_p, negative_p, false});
}

void LineManager::key_pressed(int key_p)
{
}

void LineManager::spawn_splitter_internal(int x, int y, bool horizontal_p, bool negative_p, bool flipped_p)
{
	Position pos_in_l {x,y};
	auto &&pair_in_l = create_unit_line(horizontal_p, negative_p, ecs, pos_in_l);
	flecs::entity line_in_l = pair_in_l.first;
	Position pos_in_line_from_spot_l = pair_in_l.second;
	Position pos_out_1_l {x,y};
	Position pos_out_2_l {x,y};
	if(horizontal_p)
	{
		if(negative_p)
		{
			pos_out_1_l.x -= 1;
			pos_out_2_l.x -= 1;
		}
		else
		{
			pos_out_1_l.x += 1;
			pos_out_2_l.x += 1;
		}
		if(flipped_p)
		{
			pos_out_2_l.y += 1;
		}
		else
		{
			pos_out_2_l.y -= 1;
		}
	}
	else
	{
		if(negative_p)
		{
			pos_out_1_l.y -= 1;
			pos_out_2_l.y -= 1;
		}
		else
		{
			pos_out_1_l.y += 1;
			pos_out_2_l.y += 1;
		}
		if(flipped_p)
		{
			pos_out_2_l.x += 1;
		}
		else
		{
			pos_out_2_l.x -= 1;
		}
	}
	flecs::entity line_out_1_l = create_unit_line(horizontal_p, negative_p, ecs, pos_out_1_l).first;
	flecs::entity line_out_2_l = create_unit_line(horizontal_p, negative_p, ecs, pos_out_2_l).first;

	if(!check_line(grid, line_in_l)
	|| !check_line(grid, line_out_1_l)
	|| !check_line(grid, line_out_2_l))
	{
		line_in_l.destruct();
		line_out_1_l.destruct();
		line_out_2_l.destruct();
		return;
	}

	add_line_display(world_size, *_drawer2, *_framesLibrary, line_in_l);
	fill(grid, line_in_l);
	add_line_display(world_size, *_drawer2, *_framesLibrary, line_out_1_l);
	fill(grid, line_out_1_l);
	add_line_display(world_size, *_drawer2, *_framesLibrary, line_out_2_l);
	fill(grid, line_out_2_l);

	flecs::entity splitter_l = create_link(ecs, "", line_in_l, line_out_1_l);
	splitter_l.set<Splitter>({line_out_2_l.get_ref<Line>()});
	line_out_2_l.set<From, Connector>({splitter_l});

	// Merging lines
	merge_around_to_pos(_drawer, grid, ecs, line_out_1_l, pos_out_1_l);
	merge_around_to_pos(_drawer, grid, ecs, line_out_2_l, pos_out_2_l);
	merge_on_from_pos(_drawer, grid, ecs, line_in_l, pos_in_line_from_spot_l);
}

void LineManager::spawn_merger_internal(int x, int y, bool horizontal_p, bool negative_p, bool flipped_p)
{
	Position pos_in_1_l {x,y};

	auto &&pair_in_l = create_unit_line(horizontal_p, negative_p, ecs, pos_in_1_l);
	flecs::entity line_in_1_l = pair_in_l.first;
	Position pos_in_line_1_from_spot_l = pair_in_l.second;

	Position pos_in_2_l {x,y};
	Position pos_out_l {x,y};
	if(horizontal_p)
	{
		if(negative_p)
		{
			pos_out_l.x -= 1;
		}
		else
		{
			pos_out_l.x += 1;
		}
		if(flipped_p)
		{
			pos_in_2_l.y += 1;
		}
		else
		{
			pos_in_2_l.y -= 1;
		}
	}
	else
	{
		if(negative_p)
		{
			pos_out_l.y -= 1;
		}
		else
		{
			pos_out_l.y += 1;
		}
		if(flipped_p)
		{
			pos_in_2_l.x += 1;
		}
		else
		{
			pos_in_2_l.x -= 1;
		}
	}

	pair_in_l = create_unit_line(horizontal_p, negative_p, ecs, pos_in_2_l);
	flecs::entity line_in_2_l = pair_in_l.first;
	Position pos_in_line_2_from_spot_l = pair_in_l.second;

	flecs::entity line_out_l = create_unit_line(horizontal_p, negative_p, ecs, pos_out_l).first;

	if(!check_line(grid, line_in_1_l)
	|| !check_line(grid, line_in_2_l)
	|| !check_line(grid, line_out_l))
	{
		line_in_1_l.destruct();
		line_in_2_l.destruct();
		line_out_l.destruct();
		return;
	}

	add_line_display(world_size, *_drawer2, *_framesLibrary, line_in_1_l);
	fill(grid, line_in_1_l);
	add_line_display(world_size, *_drawer2, *_framesLibrary, line_in_2_l);
	fill(grid, line_in_2_l);
	add_line_display(world_size, *_drawer2, *_framesLibrary, line_out_l);
	fill(grid, line_out_l);

	flecs::entity merger_l = create_link(ecs, "", line_in_1_l, line_out_l);
	merger_l.set<Merger>({line_in_2_l.get_ref<Line>()});
	line_in_2_l.set<To, Connector>({merger_l});

	// Merging lines
	merge_around_to_pos(_drawer, grid, ecs, line_out_l, pos_out_l);
	merge_on_from_pos(_drawer, grid, ecs, line_in_1_l, pos_in_line_1_from_spot_l);
	merge_on_from_pos(_drawer, grid, ecs, line_in_2_l, pos_in_line_2_from_spot_l);
}

}
