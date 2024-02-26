#include "Recipe.h"

int32_t compute_full_recipe(Recipe const &recipe_p, Storer const &storer_p)
{
	int32_t recipe_qty = -1;

	for(RecipePart const &part_l : recipe_p.parts)
	{
		int32_t qty = get_quantity(storer_p, part_l.type) / part_l.qty;
		if(recipe_qty < 0 || qty < recipe_qty)
		{
			recipe_qty = qty;
		}
	}
	return std::max(recipe_qty, 0);
}

double compute_value(Recipe const &recipe_p, Storer const &storer_p)
{
	return compute_full_recipe(recipe_p, storer_p) * recipe_p.value;
}

Storer compute_residuals(Recipe const &recipe_p, Storer const &storer_p)
{
	Storer storer_l = storer_p;
	int32_t full_recipe_l = compute_full_recipe(recipe_p, storer_p);

	for(RecipePart const &part_l : recipe_p.parts)
	{
		storer_l.quantities[part_l.type] -= part_l.qty * full_recipe_l;
	}
	return storer_l;
}
