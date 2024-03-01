class_name NewRecipeBonus extends Bonus

var template : RecipeTemplate

func apply_to_run(run : RunInfo):
	run.get_or_add_recipe_template(template)

static func gen_template(run : RunInfo) -> RecipeTemplate:
	var qty_total = randi_range(2,4)

	var new_template = RecipeTemplate.new()
	new_template.value = (randi_range(9,14) + qty_total) * qty_total

	var types = []
	for spawn in run.spawns:
		for type in spawn.types:
			if not types.has(type):
				types.append(type)

	var selected_types : Array[int] = []
	var selected_qties : Array[int] = []
	var remaining_qty = qty_total
	while remaining_qty > 0 and types.size() > 0:
		# qty
		var qty_rolled = randi_range(1,min(remaining_qty,2))
		remaining_qty -= qty_rolled
		# type
		var type_rolled = types[randi_range(0, types.size()-1)]
		types.erase(type_rolled)

		selected_types.append(type_rolled)
		selected_qties.append(qty_rolled)

	new_template.types = selected_types
	new_template.quantities = selected_qties

	return new_template

static func gen_bonus(run : RunInfo) -> NewRecipeBonus:
	var new_bonus = NewRecipeBonus.new()

	new_bonus.template = gen_template(run)

	return new_bonus
