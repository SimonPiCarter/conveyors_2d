#pragma once

#include <godot_cpp/godot.hpp>
#include <godot_cpp/classes/node2d.hpp>

#include <flecs.h>
#include <thread>
#include <random>
#include <vector>
#include <mutex>
#include "entity_drawer/EntityDrawer.h"
#include "entity_drawer/FramesLibrary.h"

#include "lib/factories/Connector.h"
#include "lib/factories/Line.h"
#include "lib/factories/Drawable.h"
#include "lib/grid/Grid.h"
#include "lib/level/Level.h"

namespace godot {

class LineManager : public Node2D {
	GDCLASS(LineManager, Node2D)

public:
	~LineManager();

	void init(int seed_p);
	void loop();
	void _process(double delta_p) override;

	// Will be called by Godot when the class is registered
	// Use this to add properties to your class
	static void _bind_methods();

	void setEntityDrawer(EntityDrawer *drawer_p);
	EntityDrawer *getEntityDrawer() const;

	void setEntityDrawer2(EntityDrawer *drawer_p);
	EntityDrawer *getEntityDrawer2() const;

	void setFramesLibrary(FramesLibrary *lib_p);
	FramesLibrary *getFramesLibrary() const;

	float get_world_size() const { return world_size; }

	void spawn_line(int x, int y, bool honrizontal_p, bool negative_p);
	void spawn_splitter(int x, int y, bool horizontal_p, bool negative_p, bool flipped_p);
	void spawn_merger(int x, int y, bool horizontal_p, bool negative_p, bool flipped_p);

	/// DEBUG
	void key_pressed(int key_p);
	double get_score();

private:
	void spawn_splitter_internal(int x, int y, bool horizontal_p, bool negative_p, bool flipped_p);
	void spawn_merger_internal(int x, int y, bool horizontal_p, bool negative_p, bool flipped_p);
	std::thread * _thread = nullptr;

	bool _init = false;
	double _elapsed = 0.;

	float world_size = 24;
	double time_step = 0.1;
	uint32_t _timestamp = 0;

	std::mt19937 *_gen = nullptr;

	flecs::world ecs;
	flecs::query<Line const, flecs::pair<From, Position>, flecs::pair<To, Position>> update_display;
	flecs::query<DrawingInit const> init_display;
	flecs::query<Drawing const, Consumed const> consumed_objects;
	Grid grid = {512, 512};

	EntityDrawer * _drawer = nullptr;
	EntityDrawer * _drawer2 = nullptr;
	FramesLibrary * _framesLibrary = nullptr;

	// level info
	Level level;

	// spawned queues
	std::list<SpawnLine> _line_spawn_queue;
	std::list<SpawnSplitter> _splitter_spawn_queue;
	std::list<SpawnMerger> _merger_spawn_queue;

	// spawned line systems
	flecs::system new_line_system;
	flecs::system merge_line_system;
};

}
