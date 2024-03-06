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
@export var spawner_manager : SpawnerManager = null
# @export var running_phase : RunningStage = null

func set_phase(new_phase : Phase):
	if new_phase == current_phase:
		return

	if build_phase != null:
		build_phase.process_mode = Node.PROCESS_MODE_DISABLED if new_phase != Phase.BUILDING else Node.PROCESS_MODE_INHERIT
		build_phase.visible = new_phase == Phase.BUILDING
		if new_phase == Phase.BUILDING:
			build_phase.init()

	line_manager.set_build_phase(new_phase == Phase.BUILDING)
	line_manager.set_paused(new_phase != Phase.RUNNING)

	# entering building
	if new_phase == Phase.BUILDING:
		run_info.init(line_manager, spawner_manager)

	# entering running
	if new_phase == Phase.RUNNING:
		run_info.round_count += 1
		run_info.init(line_manager, spawner_manager)

	# entering score
	if new_phase == Phase.SCORE:
		score_phase.show()
		score_phase.set_score(line_manager.get_score())

	# entering shop
	if new_phase == Phase.SHOP:
		if run_info:
			shop_phase.shop_panel.gen_shop_items(run_info)
		shop_phase.show()

	# leaving running
	if current_phase == Phase.RUNNING:
		line_manager.clear_all()

	# leaving score
	if current_phase == Phase.SCORE:
		score_phase.hide()

	# leaving shop
	if current_phase == Phase.SHOP:
		shop_phase.hide()

	current_phase = new_phase

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
