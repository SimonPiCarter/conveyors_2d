extends Node2D

@onready var entity_drawer = $EntityDrawer
@onready var frames_library = $FramesLibrary
@onready var line_manager = $LineManager

# Called when the node enters the scene tree for the first time.
func _ready():
	entity_drawer.set_time_step(0.1)

	frames_library.addFrame("blue", preload("res://godot/frames/jams/blue.tres"), Vector2(0,0), false)
	frames_library.addFrame("green", preload("res://godot/frames/jams/green.tres"), Vector2(0,0), false)
	frames_library.addFrame("pink", preload("res://godot/frames/jams/pink.tres"), Vector2(0,0), false)
	frames_library.addFrame("red", preload("res://godot/frames/jams/red.tres"), Vector2(0,0), false)
	frames_library.addFrame("yellow", preload("res://godot/frames/jams/yellow.tres"), Vector2(0,0), false)
	frames_library.addFrame("belt", preload("res://godot/frames/belt/belt_v1.tres"), Vector2(0,0), false)

	line_manager.setEntityDrawer(entity_drawer)
	line_manager.setFramesLibrary(frames_library)

	line_manager.init()

func _input(event):
	if event is InputEventKey and event.is_pressed():
		line_manager.key_pressed(event.keycode)
