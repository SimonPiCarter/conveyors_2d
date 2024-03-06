class_name ImproveRecipeBonus extends Bonus

var template : RecipeTemplate
var new_value : float = 0

func apply_to_run(run : RunInfo):
	var run_template = run.get_or_add_recipe_template(template)
	run_template.value = new_value

static func gen_bonus(run : RunInfo) -> ImproveRecipeBonus:
	var new_bonus = ImproveRecipeBonus.new()

	var idx = randi_range(0, run.recipe_library.size()-1)
	new_bonus.template = run.recipe_library.values()[idx]
	new_bonus.new_value = new_bonus.template.value
	new_bonus.new_value *= 1.+randi_range(1,5)/10.

	return new_bonus
