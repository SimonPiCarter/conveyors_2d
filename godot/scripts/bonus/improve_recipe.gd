class_name ImproveRecipeBonus extends Bonus

var template : RecipeTemplate
var add_value : float = 0
var mult_value : float = 1

func apply_to_run(run : RunInfo):
	var run_template = run.get_or_add_recipe_template(template)
	run_template.value += add_value
	run_template.value *= mult_value

static func gen_bonus(run : RunInfo) -> ImproveRecipeBonus:
	var new_bonus = ImproveRecipeBonus.new()

	var idx = randi_range(0, run.recipe_library.size()-1)
	new_bonus.template = run.recipe_library.values()[idx]

	if randi_range(0,1) > 0:
		new_bonus.add_value = randi_range(1,4)
	else:
		new_bonus.mult_value = 1.+randi_range(1,5)/10.

	return new_bonus
