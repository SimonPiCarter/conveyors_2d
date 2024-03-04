#include "LineManager.h"

#include <godot_cpp/variant/utility_functions.hpp>

#include <chrono>
#include <sstream>
#include <cstdlib>

#include "lib/pipeline/PipelineSteps.h"
#include "lib/line/systems/LineSystems.h"
#include "lib/drawing/systems/DrawingSystems.h"

namespace godot {


LineManager::~LineManager()
{
	delete _thread;
	delete _gen;
}

std::string get_belt_name(flecs::entity ent)
{
	CellLine const *line_l = ent.get<CellLine>();

	if(line_l)
	{
		if(line_l->start.x > line_l->end.x)
		{
			return "belt_left";
		}
		if(line_l->start.x < line_l->end.x)
		{
			return "belt_right";
		}
		if(line_l->start.y < line_l->end.y)
		{
			return "belt_down";
		}
		if(line_l->start.y > line_l->end.y)
		{
			return "belt_up";
		}
	}

	return "belt";
}

void LineManager::init(int seed_p)
{
	if(_init) { return; }
	UtilityFunctions::print("init");

	delete _gen;
	_gen = new std::mt19937(seed_p);

	//// >LEVEL INSTANCE

	grid.set(10, 8, create_up(ecs, 10, 8));
	grid.set(10, 10, create_up(ecs, 10, 10));
	grid.set(10, 9, create_up(ecs, 10, 9));

	grid.set(13, 11, create_left(ecs, 13, 11));
	grid.set(11, 11, create_left(ecs, 11, 11));
	grid.set(12, 11, create_left(ecs, 12, 11));

	grid.set(14, 10, create_down(ecs, 14, 10));
	grid.set(14, 9, create_down(ecs, 14, 9));
	grid.set(14, 8, create_down(ecs, 14, 8));

	flecs::entity ent = create_empty(ecs, 10, 11);
	create_cell_half_line_right(ecs, ent, true);
	create_cell_half_line_up(ecs, ent, false);

	ent = create_empty(ecs, 14, 11);
	create_cell_half_line_left(ecs, ent, false);
	create_cell_half_line_up(ecs, ent, true);

	merge_all_cells(ecs, grid);
	create_all_lines(ecs);
	link_all_simple_cells(ecs);

	flecs::entity cell = grid.get(14, 8);
	Cell const * cell_component = cell.get<Cell>();
	flecs::entity cell_line = cell_component->lines[0];
	cell_line.set<Spawn>({{0}, 0, 0});

	tag_all_magnitude(ecs);

	// systems

	set_up_line_systems(ecs, _timestamp, world_size, *_gen, false);
	set_up_display_systems(ecs, this);


	// Create custom pipeline
	iteration_pipeline = ecs.pipeline()
		.with(flecs::System)
		.with<Iteration>()
		.build();

	display_pipeline = ecs.pipeline()
		.with(flecs::System)
		.with<Display>()
		.build();

	if(_drawer)
	{
		_drawer->set_time_step(time_step);
	}
	if(_drawer2)
	{
		_drawer2->set_time_step(time_step);
	}

	UtilityFunctions::print("done");
	_init = true;
}

void LineManager::loop()
{
	auto start{std::chrono::steady_clock::now()};

	ecs.set_pipeline(iteration_pipeline);
	ecs.progress();

	auto end{std::chrono::steady_clock::now()};
	std::chrono::duration<double> diff = end - start;

	// UtilityFunctions::print("total ", diff.count()*1000.);
}

void LineManager::_process(double delta)
{
	Node::_process(delta);
	if(!_init || _paused || _build_phase)
	{
		return;
	}
	_elapsed += delta;

	if(_elapsed >= time_step
	&& (_timestamp < _max_timestamp || _max_timestamp == 0))
	{
		_elapsed -= time_step;
		++_timestamp;
		// finish old loop
		if(_thread)
		{
			_thread->join();

			getEntityDrawer()->update_pos();
			getEntityDrawer2()->update_pos();
			ecs.set_pipeline(display_pipeline);
			ecs.progress();
		}

		// new loop
		delete _thread;
		_thread = new std::thread(&LineManager::loop, this);
	}
}

void LineManager::_bind_methods()
{
	ClassDB::bind_method(D_METHOD("init", "seed"), &LineManager::init);
	ClassDB::bind_method(D_METHOD("setEntityDrawer", "drawer"), &LineManager::setEntityDrawer);
	ClassDB::bind_method(D_METHOD("getEntityDrawer"), &LineManager::getEntityDrawer);
	ClassDB::bind_method(D_METHOD("setEntityDrawer2", "drawer"), &LineManager::setEntityDrawer2);
	ClassDB::bind_method(D_METHOD("getEntityDrawer2"), &LineManager::getEntityDrawer2);
	ClassDB::bind_method(D_METHOD("setFramesLibrary", "library"), &LineManager::setFramesLibrary);
	ClassDB::bind_method(D_METHOD("getFramesLibrary"), &LineManager::getFramesLibrary);
	ClassDB::bind_method(D_METHOD("get_world_size"), &LineManager::get_world_size);
	ClassDB::bind_method(D_METHOD("spawn_line", "x", "y", "horizontal", "negative"), &LineManager::spawn_line);
	ClassDB::bind_method(D_METHOD("remove_line", "x", "y"), &LineManager::remove_line);
	ClassDB::bind_method(D_METHOD("spawn_splitter", "x", "y", "horizontal", "negative", "flipped"), &LineManager::spawn_splitter);
	ClassDB::bind_method(D_METHOD("spawn_merger", "x", "y", "horizontal", "negative", "flipped"), &LineManager::spawn_merger);
	ClassDB::bind_method(D_METHOD("add_spawn_to_line", "x", "y", "types", "spawn_time"), &LineManager::add_spawn_to_line);
	ClassDB::bind_method(D_METHOD("add_recipe_and_storer_to_line", "x", "y", "types", "qty", "value"), &LineManager::add_recipe_and_storer_to_line);

	ClassDB::bind_method(D_METHOD("set_max_timestamp", "timestamp_"), &LineManager::set_max_timestamp);
	ClassDB::bind_method(D_METHOD("get_max_timestamp"), &LineManager::get_max_timestamp);
	ClassDB::bind_method(D_METHOD("get_timestamp"), &LineManager::get_timestamp);
	ClassDB::bind_method(D_METHOD("is_over"), &LineManager::is_over);
	ClassDB::bind_method(D_METHOD("set_paused", "paused"), &LineManager::set_paused);
	ClassDB::bind_method(D_METHOD("is_paused"), &LineManager::is_paused);
	ClassDB::bind_method(D_METHOD("set_build_phase", "build_phase"), &LineManager::set_build_phase);
	ClassDB::bind_method(D_METHOD("is_build_phase"), &LineManager::is_build_phase);
	ClassDB::bind_method(D_METHOD("clear_all"), &LineManager::clear_all);

	ClassDB::bind_method(D_METHOD("key_pressed", "key"), &LineManager::key_pressed);
	ClassDB::bind_method(D_METHOD("get_score"), &LineManager::get_score);

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

void LineManager::spawn_line(int x, int y, bool horizontal_p, bool negative_p)
{
}

void LineManager::remove_line(int x, int y)
{
}

void LineManager::spawn_splitter(int x, int y, bool horizontal_p, bool negative_p, bool flipped_p)
{
}

void LineManager::spawn_merger(int x, int y, bool horizontal_p, bool negative_p, bool flipped_p)
{
}

void LineManager::add_spawn_to_line(int x, int y, TypedArray<int> const &types_p, int spawn_time_p)
{
}

void LineManager::add_recipe_and_storer_to_line(int x, int y, TypedArray<int> const &types_p, TypedArray<int> const &qty_p, double value_p)
{
}

void LineManager::set_max_timestamp(int timestamp_p)
{
	_max_timestamp = timestamp_p;
}

int LineManager::get_max_timestamp()
{
	return _max_timestamp;
}

int LineManager::get_timestamp()
{
	return _timestamp;
}

bool LineManager::is_over()
{
	return _timestamp >= _max_timestamp;
}

void LineManager::set_paused(bool paused_p)
{
	_paused = paused_p;
}

bool LineManager::is_paused()
{
	return _paused;
}

void LineManager::set_build_phase(bool build_phase_p)
{
	_build_phase = build_phase_p;
}

bool LineManager::is_build_phase()
{
	return _build_phase;
}

void LineManager::clear_all()
{
	_timestamp = 0;
	_elapsed = 0;
}

void LineManager::key_pressed(int key_p)
{
}

double LineManager::get_score()
{
	double score_l = 0;
	return score_l;
}

}
