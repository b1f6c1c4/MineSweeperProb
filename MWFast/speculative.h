#pragma once
#include "block.h"
#include "grid.h"

class spec_grid_t : public grid_t<blk_t>
{
public:
	spec_grid_t(const grid_t<blk_t> &o, rep_t r);

	rep_t repitition;
};
