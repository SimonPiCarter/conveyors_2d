class_name ShopStage extends Control

@onready var button = $Button
@onready var shop_panel = $shop_panel

signal end_shop_stage

var run_info : RunInfo = null

func _ready():
	button.pressed.connect(emit_signal.bind("end_shop_stage"))
	shop_panel.selected_item.connect(buy_item)

func buy_item(bonus):
	if(run_info):
		bonus.apply_to_run(run_info)
	end_shop_stage.emit()

