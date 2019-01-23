#pragma once
#include "common.h"
#include "basic_logic.h"
#include "speculative.h"

class full_logic : protected basic_logic
{
public:
	struct fork_directive
	{
		size_t index;
		const grid_t<blk_t> *base;
		bool value;
	};

	full_logic(grid_t<blk_t> &grid, std::shared_ptr<logic_config> config);
	logic_result try_full_logics(bool force = false);

	std::vector<spec_grid_t> spec_grids_;
private:

	logic_result try_full_logic(bool force = false);
	void prepare_full_logic();
	void speculative_fork(fork_directive directive);

	grid_t<blk_t> grid_;
	std::vector<blk_ref> front_set_;
};
