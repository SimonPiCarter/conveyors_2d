class_name NewLineBonus extends Bonus

var spawn : SpawnInfo
var recipe : RecipeInfo

func apply_to_run(run : RunInfo):
	spawn.x = (1+run.spawns.size()) * run.step_spawn
	spawn.y = 0
	recipe.x = spawn.x
	recipe.y = 25

	run.spawns.append(spawn)
	run.recipes.append(recipe)

static func gen_bonus(run : RunInfo) -> NewLineBonus:
	var new_line = NewLineBonus.new()

	var types : Array[int] = []
	types.append(randi_range(0,4))
	var quantities : Array[int] = []
	quantities.append(1)

	new_line.spawn = SpawnInfo.new()
	new_line.spawn.x = 0
	new_line.spawn.y = 0
	new_line.spawn.types = types
	new_line.spawn.spawn_time = 5

	new_line.recipe = RecipeInfo.new()
	new_line.recipe.x = 0
	new_line.recipe.y = 0

	var template = RecipeTemplate.new()
	template.types = types
	template.quantities = quantities
	template.value = randi_range(10,12)

	new_line.recipe.template = run.get_or_add_recipe_template(template)

	return new_line

