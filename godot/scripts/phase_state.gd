class_name PhaseState extends Node

enum Phase {
	RUNNING,
	SCORE,
	SHOP,
	BUILDING,
	START
}

var current_phase : Phase = Phase.START

var run_info : RunInfo = null

@export var build_phase : PhaseStage = null
@export var score_phase : ScoreStage = null
@export var shop_phase : ShopStage = null
@export var line_manager : LineManager = null
# @export var running_phase : RunningStage = null

func set_phase(phase : Phase):
	if phase == current_phase:
		return

	if build_phase != null:
		build_phase.process_mode = Node.PROCESS_MODE_DISABLED if phase != Phase.BUILDING else Node.PROCESS_MODE_INHERIT
		build_phase.visible = phase == Phase.BUILDING
		if phase == Phase.BUILDING:
			build_phase.init()

	line_manager.set_build_phase(phase == Phase.BUILDING)
	line_manager.set_paused(phase != Phase.RUNNING)

	# entering running
	if phase == Phase.BUILDING:
		run_info.init(line_manager)

	# leaving running
	if current_phase == Phase.RUNNING:
		line_manager.clear_all()

	current_phase = phase

static func get_phase_name(phase : Phase) -> String:
	match phase:
		Phase.RUNNING:
			return "RUNNING"
		Phase.SCORE:
			return "SCORE"
		Phase.SHOP:
			return "SHOP"
		Phase.BUILDING:
			return "BUILDING"
		Phase.START:
			return "START"
		_:
			return "unknown"
