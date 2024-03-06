extends Control

var run : RunInfo = null
var spawner_manager : SpawnerManager = null
@onready var storer_selected = $VBoxContainer/HBoxContainer/VBoxContainer/storer_selected
@onready var storer_info = $VBoxContainer/HBoxContainer/VBoxContainer/storer_info
@onready var recipe_selected = $VBoxContainer/HBoxContainer/VBoxContainer2/recipe_selected
@onready var recipe_info = $VBoxContainer/HBoxContainer/VBoxContainer2/recipe_info
@onready var save = $VBoxContainer/save

func _ready():
	storer_selected.item_selected.connect(update_storer)
	recipe_selected.item_selected.connect(update_recipe)
	save.pressed.connect(_on_save)

func init(run_in : RunInfo):
	run = run_in
	if run == null:
		return
	recipe_selected.clear()
	storer_selected.clear()
	var i = 1
	for val in run.recipe_library.values():
		recipe_selected.add_item(String.num(i))
		i += 1
	i = 1
	for val in run.recipes:
		storer_selected.add_item(String.num(i))
		i += 1

	update_storer(0)
	update_recipe(recipe_selected.selected)

func update_recipes(info : RecipeInfo):
	if run == null:
		return
	var selected = -1
	var idx = 0
	for val in run.recipe_library.values():
		if val == info.template:
			selected = idx
		idx += 1

	recipe_selected.selected = selected
	update_recipe(selected)

func get_current_storer(idx) -> RecipeInfo:
	if run == null \
	or idx < 0 \
	or idx >= run.recipes.size():
		return null
	return run.recipes[idx]

func update_storer(idx):
	var info = get_current_storer(idx)
	update_recipes(info)

	storer_info.text = "x = "+String.num(info.x, 0)+", y = "+String.num(info.y, 0)

func update_recipe(idx):
	var recipe = run.recipe_library.values()[idx]

	recipe_info.text = recipe.gen_visual_description()

func _on_save():
	if run == null:
		return
	var storer = get_current_storer(storer_selected.selected)
	if storer:
		storer.template = run.recipe_library.values()[recipe_selected.selected]
		if spawner_manager:
			spawner_manager.add_storer(storer)

