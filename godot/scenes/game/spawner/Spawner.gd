extends Node2D

@onready var desc = $desc

var info : SpawnInfo = null

func _ready():
	info = SpawnInfo.new()
	info.types = [0, 1]
	info.spawn_time = 5
	set_info(info)

func set_info(new_info : SpawnInfo):
	info = new_info
	desc.text = new_info.gen_visual_description()
