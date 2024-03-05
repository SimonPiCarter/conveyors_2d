#include "Storer.h"

int32_t get_quantity(Storer const &storer_p, int32_t type_p)
{
	auto it = storer_p.quantities.find(type_p);
	if(it != storer_p.quantities.cend())
	{
		return it->second;
	}
	return 0;
}

void add_to_storage(Storer &storer_p, int32_t type_p)
{
	++storer_p.quantities[type_p];
}
