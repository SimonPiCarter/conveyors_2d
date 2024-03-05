extends Node2D

@onready var entity_drawer = $EntityDrawer
@onready var entity_drawer_2 = $EntityDrawer2
@onready var frames_library = $FramesLibrary
@onready var line_manager = $LineManager
@onready var fps_label = $CanvasLayer/fps_label
@onready var phase_state = $PhaseState

# phases
@onready var build_phase = $build_phase
@onready var score_phase = $CanvasLayer/score_phase
@onready var shop_phase = $CanvasLayer/shop_phase

var run_info = RunInfo.new()

# Called when the node enters the scene tree for the first time.
func _ready():
	entity_drawer.set_time_step(0.1)

	frames_library.addFrame("blue", preload("res://godot/frames/jams/blue.tres"), Vector2(-10,-10), false)
	frames_library.addFrame("green", preload("res://godot/frames/jams/green.tres"), Vector2(-10,-10), false)
	frames_library.addFrame("pink", preload("res://godot/frames/jams/pink.tres"), Vector2(-10,-10), false)
	frames_library.addFrame("red", preload("res://godot/frames/jams/red.tres"), Vector2(-10,-10), false)
	frames_library.addFrame("yellow", preload("res://godot/frames/jams/yellow.tres"), Vector2(-10,-10), false)
	line_manager.set_max_timestamp(100)

	line_manager.setEntityDrawer(entity_drawer)
	line_manager.setEntityDrawer2(entity_drawer_2)
	line_manager.setFramesLibrary(frames_library)

	line_manager.set_build_phase(true)
	line_manager.set_paused(true)

	var new_line = NewLineBonus.gen_bonus(run_info)
	new_line.apply_to_run(run_info)
	new_line = NewLineBonus.gen_bonus(run_info)
	new_line.apply_to_run(run_info)
	new_line = NewLineBonus.gen_bonus(run_info)
	new_line.apply_to_run(run_info)

	run_info.init(line_manager)

	line_manager.init(42)

	build_phase.line_manager = line_manager
	build_phase.run_info = run_info
	build_phase.update_text()

	score_phase.end_score_stage.connect(phase_state.set_phase.bind(PhaseState.Phase.SHOP))

	shop_phase.run_info = run_info
	shop_phase.end_shop_stage.connect(phase_state.set_phase.bind(PhaseState.Phase.BUILDING))

	phase_state.line_manager = line_manager
	phase_state.run_info = run_info
	phase_state.run_info = run_info
	phase_state.set_phase(PhaseState.Phase.BUILDING)

func _unhandled_input(event):
	if event is InputEventKey and event.is_pressed():
		line_manager.key_pressed(event.keycode)

		# PAUSE
		if event.keycode == KEY_SPACE \
		and phase_state.current_phase == PhaseState.Phase.RUNNING:
			line_manager.set_paused(not line_manager.is_paused())

		# END BUILDING
		if event.keycode == KEY_S:
			if phase_state.current_phase == PhaseState.Phase.BUILDING:
				phase_state.set_phase(PhaseState.Phase.RUNNING)

func _process(_delta):
	var time_left = 30. - line_manager.get_timestamp()/10.
	fps_label.text = "fps "+String.num(Engine.get_frames_per_second(), 0)+\
		"\nscore "+String.num(line_manager.get_score(),0)+\
		"\ntime "+String.num(time_left,1)+"s"+\
		"\nstate : "+PhaseState.get_phase_name(phase_state.current_phase)

	if phase_state.current_phase == PhaseState.Phase.RUNNING and line_manager.is_over():
		var imp_recipe = ImproveRecipeBonus.gen_bonus(run_info)
		imp_recipe.apply_to_run(run_info)

		print(NewRecipeBonus.gen_bonus(run_info).template.gen_name())
		# TMP END

		# TODO set to SCORE
		phase_state.set_phase(PhaseState.Phase.SCORE)

