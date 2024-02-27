#pragma once

#include "flecs.h"

#include <random>
#include <vector>
#include <sstream>

#include "lib/factories/Storer.h"

struct RecipePart
{
	int32_t type = 0;
	int32_t qty = 0;
};

struct Recipe
{
	std::vector<RecipePart> parts;
	double value = 0;
};

struct RecipePack
{
	Recipe recipe;
	flecs::ref<Storer> storer;
};

double compute_value(Recipe const &recipe_p, Storer const &storer_p);

Storer compute_residuals(Recipe const &recipe_p, Storer const &storer_p);

Recipe gen_basic_recipe(std::mt19937 &gen_p, int32_t main_part_p, std::vector<int32_t> optional_parts_p, double min_value_p, double max_value_p);

std::ostream& operator<<(std::ostream& os_p, Recipe const &recipe_p);
