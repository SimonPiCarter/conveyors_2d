class_name SpawnerManager extends Node2D

var spawners = {}
var storers = {}

@export var line_manager : LineManager = null

static func get_key(info):
	return String.num(info.x)+","+String.num(info.y)

static func get_img(type):
	var color = "blue"
	match type:
		0:
			color = "blue"
		1:
			color = "red"
		2:
			color = "orange"
		3:
			color = "yellow"
		_:
			color = "green"
	return "res://godot/sprites/jams/"+color+".png"

func get_world_size():
	if line_manager:
		return line_manager.get_world_size()
	return 12

func add_spawner(info : SpawnInfo):
	var key = SpawnerManager.get_key(info)
	if not spawners.has(key):
		spawners[key] = preload("res://godot/scenes/game/spawner/Spawner.tscn").instantiate()
		add_child(spawners[key])
	spawners[key].set_info(info)
	spawners[key].position = Vector2(info.x, info.y) * get_world_size()

func add_storer(info : RecipeInfo):
	var key = SpawnerManager.get_key(info)
	if not storers.has(key):
		storers[key] = preload("res://godot/scenes/game/storer/Storer.tscn").instantiate()
		add_child(storers[key])
	storers[key].set_info(info)
	storers[key].position = Vector2(info.x, info.y) * get_world_size()
