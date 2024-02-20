extends Node2D

@onready var entity_drawer = $EntityDrawer
@onready var frames_library = $FramesLibrary

var pos = 50

# Called when the node enters the scene tree for the first time.
func _ready():
	entity_drawer.add_instance(Vector2(50,50), Vector2(0,0), preload("res://godot/frames/jams/blue.tres"), "default", "", false)
	entity_drawer.set_time_step(1./16.)

func _physics_process(delta):
	pos += 1
	entity_drawer.set_new_pos(0, Vector2(pos,50))
	entity_drawer.update_pos()
