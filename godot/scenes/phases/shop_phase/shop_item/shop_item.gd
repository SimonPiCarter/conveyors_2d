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
		label.text = "[center]ImproveRecipeBonus[/center]" \
			+ "\n" + bonus.template.gen_visual_description()
		if bonus.add_value > 0:
			label.text += "\n+ " + String.num(bonus.add_value, 2)
		if bonus.mult_value > 1:
			label.text += "\nx " + String.num(bonus.mult_value, 2)
	if bonus is NewLineBonus:
		label.text = "[center]NewLineBonus[/center]" \
			+ "\n" + bonus.spawn.gen_visual_description() \
			+ "\n\n " + bonus.recipe.template.gen_visual_description()
	if bonus is NewRecipeBonus:
		label.text = "[center]NewRecipeBonus[/center]" \
			+ "\n" + bonus.template.gen_visual_description()

func emit_selected():
	selected.emit(cur_bonus)
