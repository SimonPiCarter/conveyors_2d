extends Node2D

@onready var desc = $desc

var info : RecipeInfo = null

func _ready():
	info = RecipeInfo.new()
	info.template = RecipeTemplate.new()
	info.template.types = [0, 1]
	info.template.quantities = [2, 1]
	info.template.value = 5.21
	set_info(info)

func set_info(new_info : RecipeInfo):
	info = new_info
	desc.text = info.template.gen_visual_description()
