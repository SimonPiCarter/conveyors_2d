#pragma once

#include <vector>

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

double compute_value(Recipe const &recipe_p, Storer const &storer_p);

Storer compute_residuals(Recipe const &recipe_p, Storer const &storer_p);
