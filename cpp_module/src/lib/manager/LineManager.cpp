#include "LineManager.h"

#include <godot_cpp/variant/utility_functions.hpp>

#include <chrono>
#include <sstream>
#include <cstdlib>

#include "lib/pipeline/PipelineSteps.h"
#include "lib/line/systems/LineSystems.h"
#include "lib/line/recipe/Recipe.h"
#include "lib/line/storer/Storer.h"
#include "lib/line/Splitter.h"
#include "lib/line/Merger.h"
#include "lib/drawing/Drawing.h"
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

	// systems

	add_splitter_system(ecs);
	add_merger_system(ecs);
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

	display_init_pipeline = ecs.pipeline()
		.with(flecs::System)
		.with<DisplayInit>()
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

	// update score
	double score_l = 0;
	for(RecipePack const &pack_l : level.recipes)
	{
		flecs::ref<Storer> storer_l = pack_l.storer;
		if(storer_l.try_get())
		{
			score_l += compute_value(pack_l.recipe, *storer_l.try_get());
		}
	}
	score = score_l;

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

		// set up all lines
		if(_timestamp == 0)
		{
			// reset all
			clear_all_lines(ecs);
			add_all_cell_lines(ecs);
			merge_all_cells(ecs, grid);
			create_all_lines(ecs);
			link_all_simple_cells(ecs);
			link_all_splitter_cells(ecs);
			link_all_merger_cells(ecs);
			tag_all_magnitude(ecs);
		}

		++_timestamp;
		// finish old loop
		if(_thread)
		{
			_thread->join();

			ecs.set_pipeline(display_init_pipeline);
			ecs.progress();

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
	ClassDB::bind_method(D_METHOD("spawn_line", "x", "y", "horizontal", "negative", "sprite_frame"), &LineManager::spawn_line);
	ClassDB::bind_method(D_METHOD("spawn_turn", "x", "y", "horizontal", "negative", "flipped", "sprite_frame"), &LineManager::spawn_turn);
	ClassDB::bind_method(D_METHOD("remove_line", "x", "y"), &LineManager::remove_line);
	ClassDB::bind_method(D_METHOD("spawn_splitter", "x", "y", "horizontal", "negative", "flipped", "sprite_frame"), &LineManager::spawn_splitter);
	ClassDB::bind_method(D_METHOD("spawn_merger", "x", "y", "horizontal", "negative", "flipped", "sprite_frame"), &LineManager::spawn_merger);
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

void LineManager::spawn_line(int x, int y, bool horizontal_p, bool negative_p, Ref<SpriteFrames> const & animation_p)
{
	if(grid.get(x, y)) { return; }
	flecs::entity ent;
	if(horizontal_p && negative_p) {
		ent = create_left(ecs, x, y);
	}
	else if(horizontal_p && !negative_p) {
		ent = create_right(ecs, x, y);
	}
	else if(!horizontal_p && negative_p) {
		ent = create_up(ecs, x, y);
	}
	else if(!horizontal_p && !negative_p) {
		ent = create_down(ecs, x, y);
	}

	if(_drawer2 && animation_p.is_valid()) {
		int idx_l = _drawer2->add_instance(Vector2(x*world_size, y*world_size), Vector2(12,12), animation_p, "default", "", false);
		ent.set<Drawing>({idx_l});
	}

	grid.set(x, y, ent);
}

void LineManager::spawn_turn(int x, int y, bool horizontal_p, bool negative_p, bool flipped_p, Ref<SpriteFrames> const & animation_p)
{
	if(grid.get(x, y)) { return; }

	flecs::entity ent = create_empty(ecs, x, y);
	grid.set(x, y, ent);
	if(horizontal_p && negative_p)
	{
		create_cell_half_line_right(ecs, ent, true);
	}
	if(horizontal_p && !negative_p)
	{
		create_cell_half_line_left(ecs, ent, true);
	}
	if(!horizontal_p && negative_p)
	{
		create_cell_half_line_down(ecs, ent, true);
	}
	if(!horizontal_p && !negative_p)
	{
		create_cell_half_line_up(ecs, ent, true);
	}

	if(horizontal_p && flipped_p)
	{
		create_cell_half_line_up(ecs, ent, false);
	}
	if(horizontal_p && !flipped_p)
	{
		create_cell_half_line_down(ecs, ent, false);
	}
	if(!horizontal_p && flipped_p)
	{
		create_cell_half_line_left(ecs, ent, false);
	}
	if(!horizontal_p && !flipped_p)
	{
		create_cell_half_line_right(ecs, ent, false);
	}

	if(_drawer2 && animation_p.is_valid()) {
		int idx_l = _drawer2->add_instance(Vector2(x*world_size, y*world_size), Vector2(12,12), animation_p, "default", "", false);
		ent.set<Drawing>({idx_l});
	}
}

void LineManager::remove_line(int x, int y)
{
	flecs::entity ent = grid.get(x, y);

	if(_drawer2 && ent && ent.get<Drawing>()) {
		_drawer2->set_animation_one_shot(ent.get<Drawing>()->idx, "default");
	}

	if(ent && ent.get<Cell>()) {
		Cell const &cell_l = *ent.get<Cell>();
		for(flecs::entity line_ent : cell_l.lines) {
			line_ent.destruct();
		}
		ent.destruct();
	}
}

void LineManager::spawn_splitter(int x, int y, bool horizontal_p, bool negative_p, bool flipped_p, Ref<SpriteFrames> const & animation_p)
{
	if(grid.get(x, y)) { return; }

	flecs::entity ent = create_empty(ecs, x, y);
	grid.set(x, y, ent);
	if(horizontal_p && negative_p)
	{
		create_cell_half_line_right(ecs, ent, true);
		create_cell_half_line_left(ecs, ent, false);
	}
	if(horizontal_p && !negative_p)
	{
		create_cell_half_line_left(ecs, ent, true);
		create_cell_half_line_right(ecs, ent, false);
	}
	if(!horizontal_p && negative_p)
	{
		create_cell_half_line_down(ecs, ent, true);
		create_cell_half_line_up(ecs, ent, false);
	}
	if(!horizontal_p && !negative_p)
	{
		create_cell_half_line_up(ecs, ent, true);
		create_cell_half_line_down(ecs, ent, false);
	}

	if(horizontal_p && flipped_p)
	{
		create_cell_half_line_up(ecs, ent, false);
	}
	if(horizontal_p && !flipped_p)
	{
		create_cell_half_line_down(ecs, ent, false);
	}
	if(!horizontal_p && flipped_p)
	{
		create_cell_half_line_left(ecs, ent, false);
	}
	if(!horizontal_p && !flipped_p)
	{
		create_cell_half_line_right(ecs, ent, false);
	}

	if(_drawer2 && animation_p.is_valid()) {
		int idx_l = _drawer2->add_instance(Vector2(x*world_size, y*world_size), Vector2(12,12), animation_p, "default", "", false);
		ent.set<Drawing>({idx_l});
	}
}

void LineManager::spawn_merger(int x, int y, bool horizontal_p, bool negative_p, bool flipped_p, Ref<SpriteFrames> const & animation_p)
{
	if(grid.get(x, y)) { return; }

	flecs::entity ent = create_empty(ecs, x, y);
	grid.set(x, y, ent);
	if(horizontal_p && negative_p)
	{
		create_cell_half_line_right(ecs, ent, true);
		create_cell_half_line_left(ecs, ent, false);
	}
	if(horizontal_p && !negative_p)
	{
		create_cell_half_line_left(ecs, ent, true);
		create_cell_half_line_right(ecs, ent, false);
	}
	if(!horizontal_p && negative_p)
	{
		create_cell_half_line_down(ecs, ent, true);
		create_cell_half_line_up(ecs, ent, false);
	}
	if(!horizontal_p && !negative_p)
	{
		create_cell_half_line_up(ecs, ent, true);
		create_cell_half_line_down(ecs, ent, false);
	}

	if(horizontal_p && flipped_p)
	{
		create_cell_half_line_down(ecs, ent, true);
	}
	if(horizontal_p && !flipped_p)
	{
		create_cell_half_line_up(ecs, ent, true);
	}
	if(!horizontal_p && flipped_p)
	{
		create_cell_half_line_right(ecs, ent, true);
	}
	if(!horizontal_p && !flipped_p)
	{
		create_cell_half_line_left(ecs, ent, true);
	}

	if(_drawer2 && animation_p.is_valid()) {
		int idx_l = _drawer2->add_instance(Vector2(x*world_size, y*world_size), Vector2(12,12), animation_p, "default", "", false);
		ent.set<Drawing>({idx_l});
	}
}

void LineManager::add_spawn_to_line(int x, int y, TypedArray<int> const &types_p, int spawn_time_p)
{
	flecs::entity ent = grid.get(x, y);

	if(ent && ent.get<Cell>() && !ent.get<Cell>()->ref_lines.empty()) {
		Cell const &cell_l = *ent.get<Cell>();
		flecs::entity cell_line = cell_l.ref_lines[0];
		Spawn spawn_l = {{}, spawn_time_p, 0};
		for(size_t i = 0 ; i < types_p.size() ; ++ i) {
			spawn_l.types.push_back(types_p[i]);
		}
		cell_line.set<Spawn>(spawn_l);
	}
}

void LineManager::add_recipe_and_storer_to_line(int x, int y, TypedArray<int> const &types_p, TypedArray<int> const &qty_p, double value_p)
{
	Recipe recipe;
	for(size_t i = 0 ; i < types_p.size() && i < qty_p.size() ; ++ i) {
		recipe.parts.push_back({types_p[i], qty_p[i]});
	}
	recipe.value = value_p;

	flecs::entity storer = ecs.entity().add<Storer>();
	level.recipes.push_back({recipe, storer.get_ref<Storer>()});

	flecs::entity cell = grid.get(x, y);
	if(cell && cell.get<Cell>() && !cell.get<Cell>()->ref_lines.empty()) {
		Cell const * cell_component = cell.get<Cell>();
		flecs::entity cell_line = cell_component->ref_lines[0];
		if(cell_line) {
			cell_line.set<ConnectedToStorer>({storer});
		}
	}
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
	ecs.defer([&] {
		ecs.filter<::Object const>()
			.each([&](flecs::entity e, ::Object const&){
				e.add<Consumed>();
			});
	});

	ecs.set_pipeline(display_pipeline);
	ecs.progress();

	_elapsed = 0;
}

void LineManager::key_pressed(int key_p)
{
}

double LineManager::get_score() const
{
	return score;
}

}
