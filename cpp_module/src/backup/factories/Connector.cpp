#include "Connector.h"

#include <godot_cpp/variant/utility_functions.hpp>

void replace_connectors(flecs::entity new_p, flecs::entity_view old_p, flecs::entity connector_p)
{
	Input const * input_l = connector_p.get<Input>();
	if(input_l && input_l->prev.entity() == old_p)
	{
		connector_p.set<Input>({new_p.get_ref<Line>()});
	}

	Merger const * merger_l = connector_p.get<Merger>();
	if(merger_l && merger_l->prev.entity() == old_p)
	{
		connector_p.set<Merger>({new_p.get_ref<Line>()});
	}

	Output const * output_l = connector_p.get<Output>();
	if(output_l && output_l->next.entity() == old_p)
	{
		connector_p.set<Output>({new_p.get_ref<Line>()});
	}

	Splitter const * splitter_l = connector_p.get<Splitter>();
	if(splitter_l && splitter_l->next.entity() == old_p)
	{
		connector_p.set<Splitter>({new_p.get_ref<Line>()});
	}

	Sorter const * sorter_l = connector_p.get<Sorter>();
	if(sorter_l && sorter_l->out_type.entity() == old_p)
	{
		connector_p.set<Sorter>({new_p.get_ref<Line>(), sorter_l->out_non_type, sorter_l->type});
	}
	if(sorter_l && sorter_l->out_non_type.entity() == old_p)
	{
		connector_p.set<Sorter>({sorter_l->out_type, new_p.get_ref<Line>(), sorter_l->type});
	}
}

void remove_connector(flecs::entity connector_p)
{
	Input const * input_l = connector_p.get<Input>();
	if(input_l)
	{
		input_l->prev.entity().remove<To, Connector>();
	}

	Merger const * merger_l = connector_p.get<Merger>();
	if(merger_l)
	{
		merger_l->prev.entity().remove<To, Connector>();
	}

	Output const * output_l = connector_p.get<Output>();
	if(output_l)
	{
		output_l->next.entity().remove<From, Connector>();
	}

	Splitter const * splitter_l = connector_p.get<Splitter>();
	if(splitter_l)
	{
		splitter_l->next.entity().remove<From, Connector>();
	}

	Sorter const * sorter_l = connector_p.get<Sorter>();
	if(sorter_l)
	{
		sorter_l->out_type.entity().remove<From, Connector>();
	}
	if(sorter_l)
	{
		sorter_l->out_non_type.entity().remove<From, Connector>();
	}
}