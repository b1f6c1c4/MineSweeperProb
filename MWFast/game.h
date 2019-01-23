#pragma once
#include "common.h"
#include "grid.h"
#include "block.h"
#include "strategy.h"
#include <random>
#include "basic_logic.h"
#include "full_logic.h"

class game
{
public:
	game(size_t w, size_t h, const std::string &st);

	void fill_fixed(size_t m, bool buff = false);
	void fill_prob(double p, bool buff = false);

	const grid_t<blk_t> &grid() const;

	bool run();

	std::mt19937_64 device;
	std::shared_ptr<logic_config> config;
private:

	grid_t<blk_t> actual_;

	basic_logic basic_solver_;
	std::shared_ptr<full_logic> full_solver_;

	void two_step_logic(blk_ref b);

	grid_t<rep_t> h_mine_prob_;
	void gather_mine_prob();
	grid_t<std::array<rep_t, 8>> h_neighbor_dist_;
	grid_t<rep_t> h_zero_prob_;
	grid_t<double> h_entropy_;
	void gather_neighbor_dist(const std::vector<blk_ref> &refs);
	grid_t<rep_t> h_zeros_prob_;
	grid_t<rep_t> h_zeros_exp_;
	void gather_safe_move(const std::vector<blk_ref> &refs);

	static void initialize_mine(blk_ref b);
};
