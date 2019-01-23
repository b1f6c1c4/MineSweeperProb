#pragma once
#include "block.h"
#include "grid.h"

struct spec_stat
{
	rep_t repitition;
	rep_t probability;
	rep_t closed_rep;
	size_t closed;
	size_t total_mines;
};

typedef std::pair<grid_t<blk_t>, spec_stat> spec_grid_t;
