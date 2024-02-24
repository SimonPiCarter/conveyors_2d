#include "PositionedLine.h"

int32_t m_length(Position const & first_p, Position const & second_p)
{
	return std::abs(first_p.x - second_p.x) + std::abs(first_p.y - second_p.y);
}

PositionedLine merge_positioned_lines(RefPositionedLine const &first_p, RefPositionedLine const &second_p)
{
	Position const & first_from_l = first_p.from;
	Position const & first_to_l = first_p.to;
	Position const & second_from_l = second_p.from;
	Position const & second_to_l = second_p.to;

	bool first_horizontal_l = first_from_l.x == first_to_l.x;
	bool second_horizontal_l = first_from_l.x == first_to_l.x;

	if(first_horizontal_l != second_horizontal_l)
	{
		return PositionedLine();
	}

	Line line_l = merge_lines(first_p.line, second_p.line);
	Position from_l = first_from_l;
	Position to_l = second_to_l;

	if (m_length(from_l, to_l) != m_length(first_from_l, first_to_l) + m_length(second_from_l, second_to_l))
	{
		return PositionedLine();
	}

	return {true, line_l, from_l, to_l, first_p.co_from, second_p.co_to};
}
