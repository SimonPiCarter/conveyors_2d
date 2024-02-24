#pragma once

#include <string>

struct Drawing {
	int idx = 0;
	float x = 0;
	float y = 0;
};

struct DrawingInit {
	float x = 0;
	float y = 0;
	std::string frame;
};
