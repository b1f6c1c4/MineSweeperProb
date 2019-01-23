#pragma once
#include "common.h"
#include "grid.h"
#include "block.h"
#include "strategy.h"
#include "speculative.h"
#include <random>

typedef elem_reference<blk_t> blk_ref;

class game
{
public:
	game(size_t w, size_t h, const std::string &st);

	void fill_fixed(size_t m, bool buff = false);
	void fill_prob(double p, bool buff = false);

	const grid_t<blk_t> &grid() const;

	bool run();

	std::mt19937_64 device;
	strategy strategy;
	bool is_fixed_mines;
	size_t total_mines;
private:
	enum class logic_result
	{
		clean = 0x00,
		dirty = 0x01,
		invalid = 0x02
	};

	struct fork_directive
	{
		size_t index;
		const grid_t<blk_t> *base;
		bool value;
	};

	struct stats
	{
		size_t closed;
		size_t rest_mines;
	};

	grid_t<blk_t> actual_;
	std::vector<blk_t> speculative_;

	bool is_finished(grid_t<blk_t> &grid) const;

	void logical_open(blk_ref b);
	logic_result try_basic_logic(blk_ref b, bool aggressive);
	logic_result try_basic_logics(grid_t<blk_t> &grid);
	logic_result try_single_logic(blk_ref b, bool aggressive);
	logic_result try_ext_logic(grid_t<blk_t> &grid);
	logic_result try_full_logic();
	logic_result try_full_logics();

	stats get_stats(const grid_t<blk_t> &grid) const;
	void prepare_full_logic();
	void speculative_fork(fork_directive directive);
	std::vector<blk_ref> front_set_;
	std::vector<spec_grid_t> spec_grids_;

	static void initialize_mine(blk_ref b);
};
