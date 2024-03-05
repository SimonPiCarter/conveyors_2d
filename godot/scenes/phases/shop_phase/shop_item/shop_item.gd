extends Control

@onready var label = $Label
@onready var texture_button = $TextureButton

var cur_bonus = null
signal selected(bonus)

func _ready():
	texture_button.pressed.connect(emit_selected)

func load_from_bonus(bonus : Bonus):
	cur_bonus = bonus
	if bonus is ImproveRecipeBonus:
		label.text = "ImproveRecipeBonus" \
			+ "\ntemplate = " + bonus.template.gen_description() \
			+ "\nadd_value = " + String.num(bonus.add_value, 2) \
			+ "\nmult_value = " + String.num(bonus.mult_value, 2)
	if bonus is NewLineBonus:
		label.text = "NewLineBonus" \
			+ "\nspawn x = " + String.num(bonus.spawn.x,0) \
			+ "\nspawn y = " + String.num(bonus.spawn.y,0) \
			+ "\nspawn types = " + JSON.stringify(bonus.spawn.types) \
			+ "\nspawn spawn_time = " + String.num(bonus.spawn.spawn_time,0) \
			+ "\nrecipe x = " + String.num(bonus.recipe.x,0) \
			+ "\nrecipe y = " + String.num(bonus.recipe.y,0) \
			+ "\nrecipe template = " + bonus.recipe.template.gen_description()
	if bonus is NewRecipeBonus:
		label.text = "NewRecipeBonus" \
			+ "\ntemplate = " + bonus.template.gen_description()

func emit_selected():
	print("selected")
	selected.emit(cur_bonus)
