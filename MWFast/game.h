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

	template <typename Random>
	void fill_fixed(Random &device, size_t total_mines, bool buff = false);
	template <typename Random>
	void fill_prob(Random &device, double probability, bool buff = false);

	const grid_t<blk_t> &grid() const;

	strategy strategy;
private:
	grid_t<blk_t> actual_;
	std::vector<blk_t> speculative_;

	static void initialize_mine(blk_ref b);
};

template <typename Random>
void game::fill_fixed(Random &device, size_t total_mines, const bool buff)
{
	std::uniform_int_distribution<size_t> dx(0, actual_.width() - 1);
	std::uniform_int_distribution<size_t> dy(0, actual_.height() - 1);

	while (total_mines)
	{
		const size_t x = dx(device);
		const size_t y = dy(device);

		if (strategy.initial_x == x && strategy.initial_y == y)
			continue;

		if (buff)
		{
			if (strategy.initial_x - x <= 1ull ||
				x - strategy.initial_x <= 1ull)
				if (strategy.initial_y - y <= 1ull ||
					y - strategy.initial_y <= 1ull)
					continue;
		}

		auto b = actual_(x, y);
		if (b->is_mine())
			continue;
		initialize_mine(b);
		--total_mines;
	}
}

template <typename Random>
void game::fill_prob(Random &device, const double probability, const bool buff)
{
	std::bernoulli_distribution d(probability);

	for (auto it = actual_.begin(); it != actual_.end(); ++it)
	{
		const auto x = it.x(), y = it.y();

		if (strategy.initial_x == x && strategy.initial_y == y)
			continue;

		if (buff)
		{
			if (strategy.initial_x - x <= 1ull ||
				x - strategy.initial_x <= 1ull)
				if (strategy.initial_y - y <= 1ull ||
					y - strategy.initial_y <= 1ull)
					continue;
		}

		const bool result = d(device);
		if (result)
			initialize_mine(it);
	}
}
