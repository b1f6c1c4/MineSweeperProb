#pragma once
#include "common.h"
#include "grid.h"
#include "block.h"
#include "strategy.h"
#include <random>
#include "basic_logic.h"
#include "full_logic.h"
#include "heuristic.h"

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
	std::shared_ptr<heuristic_solver> heuristic_filter_;

	void force_logic(blk_ref b);

	static void initialize_mine(blk_ref b);
};
