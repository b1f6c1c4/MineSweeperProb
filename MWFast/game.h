#pragma once
#include "common.h"
#include "grid.h"
#include "block.h"
#include <random>

typedef elem_reference<blk_t> blk_ref;

class game
{
public:
	game(size_t w, size_t h);

	template <typename Random>
	void fill_fixed(Random &device, size_t total_mines);
	template <typename Random>
	void fill_prob(Random &device, double probability);

	const grid_t<blk_t> &grid() const;

private:
	grid_t<blk_t> actual_;
	std::vector<blk_t> speculative_;

	static void initialize_mine(blk_ref b);
};

template <typename Random>
void game::fill_fixed(Random &device, size_t total_mines)
{
	std::uniform_int_distribution<> dx(0, actual_.width() - 1);
	std::uniform_int_distribution<> dy(0, actual_.height() - 1);

	while (total_mines)
	{
		const size_t x = dx(device);
		const size_t y = dy(device);
		auto b = actual_(x, y);
		if (b->is_mine())
			continue;
		initialize_mine(b);
		--total_mines;
	}
}

template <typename Random>
void game::fill_prob(Random &device, double probability)
{
	std::bernoulli_distribution d(probability);

	for (auto &b : actual_)
	{
		const bool result = d(device);
		if (result)
			initialize_mine(b);
	}
}
