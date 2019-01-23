#pragma once
#include "common.h"
#include "basic_logic.h"
#include "speculative.h"

class full_logic : protected basic_logic
{
public:
	full_logic(grid_t<blk_t> &grid, std::shared_ptr<logic_config> config);

	logic_result try_full_logics(blk_ref pivot, bool force = false);
	logic_result try_full_logics(bool force = false);

	const grid_t<blk_t> &actual() const;
	const std::vector<spec_grid_t> &spec() const;
	std::shared_ptr<logic_config> get_config() const;

private:
	struct fork_directive
	{
		size_t index;
		const grid_t<blk_t> *base;
		bool value;
	};

	logic_result try_full_logic(bool force = false);
	void prepare_full_logic();
	void speculative_fork(fork_directive directive);

	grid_t<blk_t> &grid_;
	blk_refs front_set_;
	std::vector<spec_grid_t> spec_grids_;
};
