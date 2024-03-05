class_name PhaseStage extends Node2D

@onready var label = $CanvasLayer/Label
@onready var build_overlay = $build_overlay
@onready var recipe_selector = $CanvasLayer/Panel/recipe_selector

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
	elif mode == BuildMode.Mode.TURN:
		type_str = "Turn"
	label.text = \
		("horizontal" if horizontal else "vertical") + "\n" + \
		("negative" if negative else "positive") + "\n" + \
		("flipped" if flipped else "non-flipped") + "\n" + \
		type_str
	build_overlay.update_overlay(mode, horizontal, negative, flipped)

func handle_clic(x, y):
	var frame = BeltFrameHelper.getBeltFrame(mode, horizontal, negative, flipped)
	if mode == BuildMode.Mode.MERGER:
		line_manager.spawn_merger(x, y , horizontal, negative, flipped, frame)
	elif mode == BuildMode.Mode.SPLITTER:
		line_manager.spawn_splitter(x, y , horizontal, negative, flipped, frame)
	elif mode == BuildMode.Mode.TURN:
		line_manager.spawn_turn(x, y , horizontal, negative, flipped, frame)
	else:
		line_manager.spawn_line(x, y , horizontal, negative, frame)

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
		if event.keycode == KEY_Q:
			mode = BuildMode.Mode.TURN

		update_text()

	if event is InputEventMouseButton and event.is_pressed() and event.button_index == MOUSE_BUTTON_LEFT:
		handle_clic(build_overlay.x, build_overlay.y)

	if event is InputEventMouseButton and event.is_pressed() and event.button_index == MOUSE_BUTTON_RIGHT:
		line_manager.remove_line(build_overlay.x, build_overlay.y)

	if event is InputEventMouseMotion and event.button_mask & MOUSE_BUTTON_MASK_LEFT:
		handle_clic(build_overlay.x, build_overlay.y)

	# if event is InputEventMouseMotion and event.button_mask & MOUSE_BUTTON_MASK_RIGHT:
	# 	line_manager.remove_line(build_overlay.x, build_overlay.y)

func init():
	recipe_selector.init(run_info)
