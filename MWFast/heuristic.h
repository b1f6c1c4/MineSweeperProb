#pragma once
#include "common.h"
#include "full_logic.h"

class heuristic_solver
{
public:
	explicit heuristic_solver(const full_logic &logic);

	void filter_p(blk_refs &next_closed, const blk_refs &closed);
	void filter_s(blk_refs &next_closed, const blk_refs &closed);
	void filter_e(blk_refs &next_closed, const blk_refs &closed);
	void filter_q(blk_refs &next_closed, const blk_refs &closed);
	void filter_z(blk_refs &next_closed, const blk_refs &closed);

private:
	const full_logic &logic_;

	bool gathered_mine_prob_;
	grid_t<rep_t> h_mine_prob_;
	void gather_mine_prob();

	bool gathered_neighbor_dist;
	grid_t<std::array<rep_t, 8>> h_neighbor_dist_;
	grid_t<rep_t> h_zero_prob_;
	grid_t<double> h_entropy_;
	void gather_neighbor_dist(const blk_refs &refs);

	bool gathered_safe_move;
	grid_t<rep_t> h_zeros_prob_;
	grid_t<rep_t> h_zeros_exp_;
	void gather_safe_move(const blk_refs &refs);
};