#pragma once
#include "common.h"
#include "base_logic.h"

class basic_logic
{
public:
	explicit basic_logic(std::shared_ptr<logic_config> config);

	logic_result try_basic_logic(blk_ref b, bool aggressive) const;
	logic_result try_basic_logics(grid_t<blk_t> &grid) const;
	logic_result try_single_logic(blk_ref b, bool aggressive) const;
	logic_result try_dual_logic(blk_ref b1, blk_ref b2) const;
	logic_result try_ext_logic(grid_t<blk_t> &grid) const;

	stats get_stats(const grid_t<blk_t> &grid) const;

	bool is_finished(grid_t<blk_t> &grid) const;

protected:
	std::shared_ptr<logic_config> config;
};
