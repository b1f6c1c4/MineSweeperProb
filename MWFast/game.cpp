#include "game.h"
#include "binomial.h"

#ifndef NDEBUG
#include <iostream>
#define CHECK_POINT do { \
	std::cerr << actual_; \
	std::getchar(); \
} while (false)
#else
#define CHECK_POINT
#endif

game::game(const size_t w, const size_t h, const std::string &st)
	: config(std::make_shared<logic_config>(strategy_t(st), false, -1)),
	  actual_(w, h, blk_t::closed_simple(0)),
	  basic_solver_(config), full_solver_(nullptr), heuristic_filter_(nullptr) { }

const grid_t<blk_t> &game::grid() const
{
	return actual_;
}

void game::fill_fixed(size_t m, const bool buff)
{
	config->is_fixed_mines = true;
	config->total_mines = m;

	const std::uniform_int_distribution<size_t> dx(0, actual_.width() - 1);
	const std::uniform_int_distribution<size_t> dy(0, actual_.height() - 1);

	while (m)
	{
		const auto x = dx(device);
		const auto y = dy(device);

		if (config->strategy.initial_x == x && config->strategy.initial_y == y)
			continue;

		if (buff)
		{
			if (config->strategy.initial_x - x <= 1ull ||
				x - config->strategy.initial_x <= 1ull)
				if (config->strategy.initial_y - y <= 1ull ||
					y - config->strategy.initial_y <= 1ull)
					continue;
		}

		auto b = actual_(x, y);
		if (b->is_mine())
			continue;
		initialize_mine(b);
		--m;
	}
}

void game::fill_prob(const double p, const bool buff)
{
	config->is_fixed_mines = true;
	config->total_mines = -1;

	const std::bernoulli_distribution d(p);

	for (auto it = actual_.begin(); it != actual_.end(); ++it)
	{
		const auto x = it.x(), y = it.y();

		if (config->strategy.initial_x == x && config->strategy.initial_y == y)
			continue;

		if (buff)
		{
			if (config->strategy.initial_x - x <= 1ull ||
				x - config->strategy.initial_x <= 1ull)
				if (config->strategy.initial_y - y <= 1ull ||
					y - config->strategy.initial_y <= 1ull)
					continue;
		}

		const auto result = d(device);
		if (result)
			initialize_mine(it);
	}
}

bool game::run()
{
	auto init = actual_(config->strategy.initial_x, config->strategy.initial_y);
	if (init->is_closed())
	{
		if (!init->is_closed())
			throw std::runtime_error("Internal error: try to open a opened block");

		if (init->is_mine())
			throw std::runtime_error("Internal error: try to logically open a mine");

		init->set_closed(false);
		force_logic(init);
	}

	if (basic_solver_.is_finished(actual_))
		return true;

	if (!config->strategy.heuristic_enabled)
		return false;

	do
	{
		auto b = heuristic_select();

#ifndef NDEBUG
		for (auto &bb : actual_)
			bb.set_front(false);
		b->set_front(true);
		CHECK_POINT;
#endif

		if (b->is_mine())
			return false;

		b->set_closed(false);
		force_logic(b);
	}
	while (!basic_solver_.is_finished(actual_));

	return true;
}

void game::force_logic(const blk_ref b)
{
	full_solver_ = std::make_shared<full_logic>(actual_, config);
	FORCE_LOGIC(full_solver_->try_full_logics(b, false));
}

blk_ref game::heuristic_select()
{
	heuristic_filter_ = std::make_shared<heuristic_solver>(*full_solver_);

	blk_refs closed;
	for (auto it = actual_.begin(); it != actual_.end(); ++it)
		if (it->is_closed())
			closed.push_back(it);

	for (auto m : config->strategy.decision_tree)
	{
		blk_refs next_closed;
		switch (m)
		{
		case strategy_t::heuristic_method::min_mine_prob:
			heuristic_filter_->filter_p(next_closed, closed);
			break;
		case strategy_t::heuristic_method::max_zeros_prob:
			heuristic_filter_->filter_s(next_closed, closed);
			break;
		case strategy_t::heuristic_method::max_zeros_exp:
			heuristic_filter_->filter_e(next_closed, closed);
			break;
		case strategy_t::heuristic_method::max_entropy:
			heuristic_filter_->filter_q(next_closed, closed);
			break;
		case strategy_t::heuristic_method::max_zero_prob:
			heuristic_filter_->filter_z(next_closed, closed);
			break;
		default:
			throw std::runtime_error("Internal error: heuristic method not supported");
		}

		closed = std::move(next_closed);
		if (closed.empty())
			throw std::runtime_error("Internal error: all blocks filtered");
		if (closed.size() == 1)
			break;
	}

	const std::uniform_int_distribution<size_t> dist(0, closed.size() - 1);
	auto b = closed[dist(device)];
	if (!b->is_closed())
		throw std::runtime_error("Internal error: try to open a opened block");
	return b;
}

void game::initialize_mine(blk_ref b)
{
	b->set_mine(true);
	for (auto &n : b.neighbors())
		if (!n->is_mine())
			++*n;
}
