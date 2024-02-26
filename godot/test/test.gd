extends Node2D

@onready var entity_drawer = $EntityDrawer
@onready var entity_drawer_2 = $EntityDrawer2
@onready var frames_library = $FramesLibrary
@onready var line_manager = $LineManager
@onready var label = $CanvasLayer/Label

var horizontal = true
var negative = false

# Called when the node enters the scene tree for the first time.
func _ready():
	entity_drawer.set_time_step(0.1)

	frames_library.addFrame("blue", preload("res://godot/frames/jams/blue.tres"), Vector2(0,0), false)
	frames_library.addFrame("green", preload("res://godot/frames/jams/green.tres"), Vector2(0,0), false)
	frames_library.addFrame("pink", preload("res://godot/frames/jams/pink.tres"), Vector2(0,0), false)
	frames_library.addFrame("red", preload("res://godot/frames/jams/red.tres"), Vector2(0,0), false)
	frames_library.addFrame("yellow", preload("res://godot/frames/jams/yellow.tres"), Vector2(0,0), false)
	frames_library.addFrame("belt", preload("res://godot/frames/belt/belt_v1.tres"), Vector2(12,12), false)

	line_manager.setEntityDrawer(entity_drawer)
	line_manager.setEntityDrawer2(entity_drawer_2)
	line_manager.setFramesLibrary(frames_library)

	line_manager.init()

	label.text = \
		("horizontal" if horizontal else "vertical") + "\n" + \
		("negative" if negative else "positive")

func _input(event):
	if event is InputEventKey and event.is_pressed():
		line_manager.key_pressed(event.keycode)
		if event.keycode == KEY_H:
			horizontal = not horizontal
		if event.keycode == KEY_N:
			negative = not negative

		label.text = \
			("horizontal" if horizontal else "vertical") + "\n" + \
			("negative" if negative else "positive")

	if event is InputEventMouseButton and event.is_pressed() and event.button_index == MOUSE_BUTTON_LEFT:
		var x = int(event.global_position.x/line_manager.get_world_size())
		var y = int(event.global_position.y/line_manager.get_world_size())
		line_manager.spawn_line(x, y , horizontal, negative)

