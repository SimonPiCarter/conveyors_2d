class_name RunInfo

var spawns : Array[SpawnInfo] = []
var recipes : Array[RecipeInfo] = []
# distance between every spawn
var step_spawn = 5

var recipe_library : Dictionary = {}

func get_or_add_recipe_template(template : RecipeTemplate) -> RecipeTemplate:
	if not recipe_library.has(template.gen_name()):
		recipe_library[template.gen_name()] = template
	return recipe_library[template.gen_name()]

func init(line_manager):
	for spawn in spawns:
		for i in range(0,3):
			line_manager.spawn_line(spawn.x,spawn.y+i,false,false);
		line_manager.add_spawn_to_line(spawn.x, spawn.y, spawn.types, spawn.spawn_time)

	for recipe in recipes:
		for i in range(0,3):
			line_manager.spawn_line(recipe.x,recipe.y-i,false,false);
		line_manager.add_recipe_and_storer_to_line(recipe.x, recipe.y, recipe.template.types, recipe.template.quantities, recipe.template.value)

func print_run():
	for spawn in spawns:
		print("spawn :")
		print("\ttypes : ",spawn.types)
	for recipe in recipes:
		print("recipe :")
		print("\ttypes : ",recipe.template.types)
		print("\tqties : ",recipe.template.quantities)
		print("\tvalue : ",recipe.template.value)
