extends Node2D

@onready var label = $CanvasLayer/Label
@onready var build_overlay = $build_overlay
var line_manager : LineManager = null
var run_info : RunInfo = null

var sequence_horizontal = [false, true, false, true]
var sequence_negative = [false, true, true, false]
var sequence_index = 0

var horizontal = false
var negative = false
var flipped = false

var mode : BuildMode.Mode = BuildMode.Mode.LINE

func update_text():
	var type_str = "Line"
	if mode == BuildMode.Mode.MERGER:
		type_str = "Merger"
	elif mode == BuildMode.Mode.SPLITTER:
		type_str = "Splitter"
	label.text = \
		("horizontal" if horizontal else "vertical") + "\n" + \
		("negative" if negative else "positive") + "\n" + \
		("flipped" if flipped else "non-flipped") + "\n" + \
		type_str
	build_overlay.update_overlay(mode, horizontal, negative, flipped)

func handle_clic(x, y):
	if mode == BuildMode.Mode.MERGER:
		line_manager.spawn_merger(x, y , horizontal, negative, flipped)
	elif mode == BuildMode.Mode.SPLITTER:
		line_manager.spawn_splitter(x, y , horizontal, negative, flipped)
	else:
		line_manager.spawn_line(x, y , horizontal, negative)

func _unhandled_input(event):
	if event is InputEventKey and event.is_pressed():
		line_manager.key_pressed(event.keycode)
		if event.keycode == KEY_R:
			sequence_index = (sequence_index + 1)%4
			horizontal = sequence_horizontal[sequence_index]
			negative = sequence_negative[sequence_index]
		if event.keycode == KEY_F:
			flipped = not flipped
		if event.keycode == KEY_A:
			mode = BuildMode.Mode.MERGER
		if event.keycode == KEY_Z:
			mode = BuildMode.Mode.SPLITTER
		if event.keycode == KEY_E:
			mode = BuildMode.Mode.LINE

		update_text()

	if event is InputEventMouseButton and event.is_pressed() and event.button_index == MOUSE_BUTTON_LEFT:
		var x = int(event.global_position.x/line_manager.get_world_size())
		var y = int(event.global_position.y/line_manager.get_world_size())
		handle_clic(x, y)

	if event is InputEventMouseButton and event.is_pressed() and event.button_index == MOUSE_BUTTON_RIGHT:
		var x = int(event.global_position.x/line_manager.get_world_size())
		var y = int(event.global_position.y/line_manager.get_world_size())
		line_manager.remove_line(x, y)

	if event is InputEventMouseMotion and event.button_mask & MOUSE_BUTTON_MASK_LEFT:
		var x = int(event.global_position.x/line_manager.get_world_size())
		var y = int(event.global_position.y/line_manager.get_world_size())
		handle_clic(x, y)
