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

Recipe gen_basic_recipe(std::mt19937 &gen_p, int32_t main_part_p, std::vector<int32_t> optional_parts_p, double min_value_p, double max_value_p)
{
	Recipe recipe_l;

	recipe_l.parts.push_back({main_part_p, 1});
	std::uniform_int_distribution<> distrib(0, 1);
	for(size_t i = 0 ; i < optional_parts_p.size() ; ++ i )
	{
		if(distrib(gen_p) > 0)
		{
			recipe_l.parts.push_back({optional_parts_p[i], 1});
		}
	}
	std::uniform_real_distribution<> distrib_real(min_value_p, max_value_p);
	recipe_l.value = std::floor(distrib_real(gen_p));

	return recipe_l;
}

double compute_pack_value(RecipePack const &pack_p)
{
	double value_l = 0;
	flecs::ref<Storer> storer_l = pack_p.storer;
	if(storer_l.try_get())
	{
		value_l = compute_value(pack_p.recipe, *storer_l.try_get());
		if(pack_p.mult_recipe.value > 1e-3)
		{
			Storer resiudals_l = compute_residuals(pack_p.recipe, *storer_l.try_get());
			double mult_l = 1. + compute_value(pack_p.mult_recipe, resiudals_l);
			value_l *= mult_l;
		}
	}
	return value_l;
}

std::ostream& operator<<(std::ostream& os_p, Recipe const &recipe_p)
{
	os_p<<"Recipe["
		<<"value= "<<recipe_p.value<<", "
		<<"parts= [";
	for(RecipePart const &part_l : recipe_p.parts)
	{
		os_p<<"Part[type= "<<part_l.type<<", qty= "<<part_l.qty<<"], ";
	}
	os_p<<"]]";
	return os_p;
}
