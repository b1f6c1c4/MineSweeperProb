#pragma once
#include "common.h"
#include "grid.h"
#include "block.h"
#include "strategy.h"
#include <random>

typedef elem_reference<blk_t> blk_ref;

class game
{
public:
	game(size_t w, size_t h, const std::string &st);

	void fill_fixed(size_t total_mines, bool buff = false);
	void fill_prob(double probability, bool buff = false);

	const grid_t<blk_t> &grid() const;

	bool run();

	std::mt19937_64 device;
	strategy strategy;
private:
	grid_t<blk_t> actual_;
	std::vector<blk_t> speculative_;

	bool is_finished() const;

	void actual_open(blk_ref b);
	bool try_basic_logic(blk_ref b, bool aggressive);
	void try_basic_logics();
	bool try_single_logic(blk_ref b, bool aggressive);
	void try_full_logic();

	void prepare_full_logic();
	size_t front_size_;

	static void initialize_mine(blk_ref b);
};
