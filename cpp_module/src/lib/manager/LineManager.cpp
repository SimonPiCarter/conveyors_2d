#include "LineManager.h"

#include <godot_cpp/variant/utility_functions.hpp>

#include <chrono>
#include <sstream>
#include <cstdlib>

#include "lib/factories/Storer.h"
#include "lib/factories/recipe/Recipe.h"
#include "lib/factories/PositionedLine.h"
#include "lib/factories/FactorySystems.h"
#include "lib/pipeline/PipelineSteps.h"

namespace godot {


LineManager::~LineManager()
{
	delete _thread;
	delete _gen;
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

std::string get_belt_name(flecs::entity ent)
{
	Position const *from_l = ent.get_second<From, Position>();
	Position const *to_l = ent.get_second<To, Position>();

	if(from_l && to_l)
	{
		if(from_l->x > to_l->x)
		{
			return "belt_left";
		}
		if(from_l->x < to_l->x)
		{
			return "belt_right";
		}
		if(from_l->y < to_l->y)
		{
			return "belt_down";
		}
		if(from_l->y > to_l->y)
		{
			return "belt_up";
		}
	}

	return "belt";
}

void add_line_display(float world_size_p, EntityDrawer &drawer_p, FramesLibrary &framesLibrary_p, flecs::entity ent)
{
	DrawingLine d_line_l;
	iterate_on_positions(ent, [&](Position const &pos_p) {
		Vector2 pos_instance_l { world_size_p * pos_p.x, world_size_p * pos_p.y - 1 };
		FrameInfo const & sprite_frame = framesLibrary_p.getFrameInfo(get_belt_name(ent));
		int idx_l = drawer_p.add_instance(pos_instance_l, sprite_frame.offset, sprite_frame.sprite_frame, "default", "", false);
		d_line_l.indexes.push_back(idx_l);
	});
	ent.set(d_line_l);
}

void LineManager::init(int seed_p)
{
	if(_init) { return; }
	UtilityFunctions::print("init");

	delete _gen;
	_gen = new std::mt19937(seed_p);

	//// >LEVEL INSTANCE
	// spawn_line(5,0,false,false);
	// spawn_line(5,1,false,false);
	// spawn_line(5,2,false,false);
	// TypedArray<int> array_l;	array_l.append(0);
	// add_spawn_to_line(5,0,array_l,0);

	// spawn_line(5,20,false,false);
	// spawn_line(5,21,false,false);
	// spawn_line(5,22,false,false);
	// TypedArray<int> array1_l;	array1_l.append(0);
	// TypedArray<int> array2_l;	array2_l.append(1);
	// add_recipe_and_storer_to_line(5,22, array1_l, array2_l, 3.);

	create_factory_systems(ecs, _timestamp, world_size, *_gen);

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

			ecs.defer([&]{
				consumed_objects.each([this](flecs::entity const &ent, Drawing const &drawing_p, Consumed const) {
					_drawer->set_animation_one_shot(drawing_p.idx, "default");
					ent.destruct();
				});
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
			spawn_line_internal(line_l.x, line_l.y, line_l.horizontal, line_l.negative);
			_line_spawn_queue.pop_front();
		}
		while(!_line_remove_queue.empty())
		{
			RemoveLine line_l = _line_remove_queue.front();
			remove_line_internal(line_l.x, line_l.y);
			_line_remove_queue.pop_front();
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
	if(!_build_phase)
	{
		_line_spawn_queue.push_back({(int32_t)x, (int32_t)y, horizontal_p, negative_p});
	}
	else
	{
		spawn_line_internal(x, y, horizontal_p, negative_p);
	}
}

void LineManager::remove_line(int x, int y)
{
	if(!_build_phase)
	{
		_line_remove_queue.push_back({(int32_t)x, (int32_t)y});
	}
	else
	{
		remove_line_internal(x, y);
	}
}

void LineManager::spawn_splitter(int x, int y, bool horizontal_p, bool negative_p, bool flipped_p)
{
	if(!_build_phase)
	{
		_splitter_spawn_queue.push_back({(int32_t)x, (int32_t)y, horizontal_p, negative_p, flipped_p});
	}
	else
	{
		spawn_splitter_internal(x, y, horizontal_p, negative_p, flipped_p);
	}
}

void LineManager::spawn_merger(int x, int y, bool horizontal_p, bool negative_p, bool flipped_p)
{
	if(!_build_phase)
	{
		_merger_spawn_queue.push_back({(int32_t)x, (int32_t)y, horizontal_p, negative_p, flipped_p});
	}
	else
	{
		spawn_merger_internal(x, y, horizontal_p, negative_p, flipped_p);
	}
}

void LineManager::add_spawn_to_line(int x, int y, TypedArray<int> const &types_p, int spawn_time_p)
{
	flecs::entity line_l = grid.get(x, y);
	if(line_l)
	{
		Spawn spawn_l;
		for(uint64_t i = 0 ; i < types_p.size() ; ++ i)
		{
			spawn_l.types.push_back(types_p[i]);
		}
		spawn_l.spawn_cooldown = spawn_time_p;
		line_l.set<Spawn>(spawn_l);
	}
}

void LineManager::add_recipe_and_storer_to_line(int x, int y, TypedArray<int> const &types_p, TypedArray<int> const &qty_p, double value_p)
{
	flecs::entity line_l = grid.get(x, y);
	if(line_l && types_p.size() == qty_p.size())
	{
		// add storer
		flecs::entity storer_l = ecs.entity()
									.add<Storer>();

		// add recipe
		Recipe recipe_l;
		for(uint64_t i = 0 ; i < types_p.size() ; ++ i)
		{
			recipe_l.parts.push_back({types_p[i], qty_p[i]});
		}
		recipe_l.value = value_p;

		level.recipes.push_back({recipe_l, storer_l.get_ref<Storer>()});

		line_l.set<ConnectedToStorer>({storer_l});
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
	for(RecipePack &pack_l : level.recipes)
	{
		Storer * store_l = pack_l.storer.try_get();
		store_l->quantities.clear();
	}

	ecs.filter<Line>().each([this](flecs::entity ent, Line& line_p) {
		clean_up_line(_drawer, ent);
		empty_line(line_p);
	});

	ecs.filter<Spawn>().each([this](flecs::entity ent, Spawn& spawn_p) {
		spawn_p.last_spawn_timestamp = 0;
	});

	_timestamp = 0;
	_elapsed = 0;
}

void LineManager::key_pressed(int key_p)
{
}

double LineManager::get_score()
{
	double score_l = 0;
	for(RecipePack &pack_l : level.recipes)
	{
		score_l += compute_value(pack_l.recipe, *pack_l.storer.try_get());
	}
	return score_l;
}

void LineManager::spawn_line_internal(int x, int y, bool honrizontal_p, bool negative_p)
{
	flecs::entity new_line_l = create_unit_line(honrizontal_p, negative_p, ecs, {x, y}).first;
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
}

void LineManager::remove_line_internal(int x, int y)
{
	flecs::entity ent_l = grid.get(x, y);
	if(ent_l)
	{
		clean_up_line(_drawer, ent_l);
		remove_display_line(_drawer2, ent_l);
		Position pos_from_l = *ent_l.get<From, Position>();
		Position pos_to_l = *ent_l.get<To, Position>();
		Line const *line_l = ent_l.get<Line>();
		size_t line_size = get_size(*line_l);
		bool horizontal_l = pos_to_l.y == pos_from_l.y;

		if(line_l && line_size == 1)
		{
			// delete line
			if(ent_l.get<From, Connector>())
			{
				unlink(ecs, ent_l);
			}
		}
		else
		{
			uint32_t size_from_l = 0;
			uint32_t size_to_l = 0;
			Position new_to_l = pos_from_l;
			Position new_from_l = pos_to_l;
			if(horizontal_l)
			{
				new_from_l.x = x;
				if(new_to_l.x < new_from_l.x)
				{
					new_to_l.x = x-1;
				}
				else
				{
					new_to_l.x = x+1;
				}
				size_from_l = new_to_l.x - pos_from_l.x;
				size_to_l = pos_to_l.x - new_from_l.x;
			}
			else
			{
				new_from_l.y = y;
				if(new_to_l.y < new_from_l.y)
				{
					new_to_l.y = y-1;
				}
				else
				{
					new_to_l.y = y+1;
				}
				size_from_l = new_to_l.y - pos_from_l.y;
				size_to_l = pos_to_l.y - new_from_l.y;
			}
			// split line

			if(size_from_l > 0)
			{
				flecs::entity from_l = ecs.entity()
						.set<From, Position>(pos_from_l)
						.set<To, Position>(new_to_l)
						.set<Line>(Line(size_from_l));
				add_line_display(world_size, *_drawer2, *_framesLibrary, from_l);
				fill(grid, from_l);
				if(ent_l.get<From, Connector>())
				{
					replace_connectors(from_l, ent_l, ent_l.get<From, Connector>()->ent.mut(ecs));
				}

				if(ent_l.get<Spawn>())
				{
					// copy seem necessary to avoid lost information
					Spawn new_spawn_l = *ent_l.get<Spawn>();
					from_l.set<Spawn>(new_spawn_l);
				}
			}
			else
			{
				unlink_from(ecs, ent_l);
			}

			if(size_to_l > 0)
			{
				flecs::entity to_l = ecs.entity()
						.set<From, Position>(new_from_l)
						.set<To, Position>(pos_to_l)
						.set<Line>(Line(size_to_l));
				add_line_display(world_size, *_drawer2, *_framesLibrary, to_l);
				fill(grid, to_l);
				if(ent_l.get<To, Connector>())
				{
					replace_connectors(to_l, ent_l, ent_l.get<To, Connector>()->ent.mut(ecs));
				}
				if(ent_l.get<ConnectedToStorer>())
				{
					// copy seem necessary to avoid lost information
					ConnectedToStorer new_storer_l = *ent_l.get<ConnectedToStorer>();
					to_l.set<ConnectedToStorer>(new_storer_l);
				}
			}
			else
			{
				unlink_to(ecs, ent_l);
			}
		}
		grid.unset(x, y);
		ent_l.destruct();
	}
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
