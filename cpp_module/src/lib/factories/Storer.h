#pragma once

#include <map>

struct Storer {
	std::map<int32_t, int32_t> quantities;
};

int32_t get_quantity(Storer const &storer_p, int32_t type_p);

void add_to_storage(Storer &storer_p, int32_t type_p);
