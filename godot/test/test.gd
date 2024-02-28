extends Node2D

@onready var entity_drawer = $EntityDrawer
@onready var entity_drawer_2 = $EntityDrawer2
@onready var frames_library = $FramesLibrary
@onready var line_manager = $LineManager
@onready var label = $CanvasLayer/Label
@onready var fps_label = $CanvasLayer/fps_label
@onready var build_overlay = $build_overlay

var run_info = RunInfo.new()

var sequence_horizontal = [false, true, false, true]
var sequence_negative = [false, true, true, false]
var sequence_index = 0

var horizontal = false
var negative = false
var flipped = false

var mode : BuildMode.Mode = BuildMode.Mode.LINE

# Called when the node enters the scene tree for the first time.
func _ready():
	entity_drawer.set_time_step(0.1)

	frames_library.addFrame("blue", preload("res://godot/frames/jams/blue.tres"), Vector2(-10,-10), false)
	frames_library.addFrame("green", preload("res://godot/frames/jams/green.tres"), Vector2(-10,-10), false)
	frames_library.addFrame("pink", preload("res://godot/frames/jams/pink.tres"), Vector2(-10,-10), false)
	frames_library.addFrame("red", preload("res://godot/frames/jams/red.tres"), Vector2(-10,-10), false)
	frames_library.addFrame("yellow", preload("res://godot/frames/jams/yellow.tres"), Vector2(-10,-10), false)
	frames_library.addFrame("belt", preload("res://godot/frames/belt/belt_v1.tres"), Vector2(0,0), false)
	frames_library.addFrame("belt_down", preload("res://godot/frames/belt/belt_down_v2.tres"), Vector2(0,0), false)
	frames_library.addFrame("belt_left", preload("res://godot/frames/belt/belt_left_v2.tres"), Vector2(0,0), false)
	frames_library.addFrame("belt_right", preload("res://godot/frames/belt/belt_right_v2.tres"), Vector2(0,0), false)
	frames_library.addFrame("belt_up", preload("res://godot/frames/belt/belt_up_v2.tres"), Vector2(0,0), false)
	frames_library.addFrame("merger_down_flipped", preload("res://godot/frames/belt/merger_down_flipped_v2.tres"), Vector2(0,0), false)
	frames_library.addFrame("merger_down", preload("res://godot/frames/belt/merger_down_v2.tres"), Vector2(0,0), false)
	frames_library.addFrame("merger_left_flipped", preload("res://godot/frames/belt/merger_left_flipped_v2.tres"), Vector2(0,0), false)
	frames_library.addFrame("merger_left", preload("res://godot/frames/belt/merger_left_v2.tres"), Vector2(0,0), false)
	frames_library.addFrame("merger_right_flipped", preload("res://godot/frames/belt/merger_right_flipped_v2.tres"), Vector2(0,0), false)
	frames_library.addFrame("merger_right", preload("res://godot/frames/belt/merger_right_v2.tres"), Vector2(0,0), false)
	frames_library.addFrame("merger_up_flipped", preload("res://godot/frames/belt/merger_up_flipped_v2.tres"), Vector2(0,0), false)
	frames_library.addFrame("merger_up", preload("res://godot/frames/belt/merger_up_v2.tres"), Vector2(0,0), false)
	frames_library.addFrame("splitter_down_flipped", preload("res://godot/frames/belt/splitter_down_flipped_v2.tres"), Vector2(0,0), false)
	frames_library.addFrame("splitter_down", preload("res://godot/frames/belt/splitter_down_v2.tres"), Vector2(0,0), false)
	frames_library.addFrame("splitter_left_flipped", preload("res://godot/frames/belt/splitter_left_flipped_v2.tres"), Vector2(0,0), false)
	frames_library.addFrame("splitter_left", preload("res://godot/frames/belt/splitter_left_v2.tres"), Vector2(0,0), false)
	frames_library.addFrame("splitter_right_flipped", preload("res://godot/frames/belt/splitter_right_flipped_v2.tres"), Vector2(0,0), false)
	frames_library.addFrame("splitter_right", preload("res://godot/frames/belt/splitter_right_v2.tres"), Vector2(0,0), false)
	frames_library.addFrame("splitter_up_flipped", preload("res://godot/frames/belt/splitter_up_flipped_v2.tres"), Vector2(0,0), false)
	frames_library.addFrame("splitter_up", preload("res://godot/frames/belt/splitter_up_v2.tres"), Vector2(0,0), false)

	line_manager.set_max_timestamp(300)

	line_manager.setEntityDrawer(entity_drawer)
	line_manager.setEntityDrawer2(entity_drawer_2)
	line_manager.setFramesLibrary(frames_library)

	line_manager.set_build_phase(true)
	line_manager.set_paused(true)

	var new_line = NewLineBonus.gen_bonus(run_info)
	new_line.apply_to_run(run_info)

	run_info.init(line_manager)

	update_text()
	line_manager.init(42)

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

func _input(event):
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
		if event.keycode == KEY_SPACE:
			if line_manager.is_over():
				line_manager.clear_all()
				line_manager.set_paused(true)
				line_manager.set_build_phase(true)

				var new_line = NewLineBonus.gen_bonus(run_info)
				new_line.apply_to_run(run_info)

				var imp_recipe = ImproveRecipeBonus.gen_bonus(run_info)
				imp_recipe.apply_to_run(run_info)

				run_info.init(line_manager)
				run_info.print_run()
			else:
				line_manager.set_paused(not line_manager.is_paused())
				print("pause ", line_manager.is_paused())
				line_manager.set_build_phase(false)

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

func _process(_delta):
	var time_left = 30. - line_manager.get_timestamp()/10.
	fps_label.text = "fps "+String.num(Engine.get_frames_per_second(), 0)+\
		"\nscore "+String.num(line_manager.get_score(),0)+\
		"\ntime "+String.num(time_left,1)+"s"
