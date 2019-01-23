#include "game.h"
#include "binomial.h"
#include <iostream>
#include <cmath>

#ifndef NDEBUG
#define CHECK_POINT do { \
	std::cerr << actual_; \
	std::getchar(); \
} while (false)
#else
#define CHECK_POINT
#endif

game::game(const size_t w, const size_t h, const std::string &st)
	: config(std::make_shared<logic_config>(strategy(st), false, -1)), actual_(w, h, blk_t::closed_simple(0)),
	  basic_solver_(config), full_solver_(nullptr),
	  h_mine_prob_(w, h, 0), h_neighbor_dist_(w, h, std::array<rep_t, 8>{}),
	  h_zero_prob_(w, h, 0), h_entropy_(w, h, 0),
	  h_zeros_prob_(w, h, 0), h_zeros_exp_(w, h, 0) { }

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

template <typename T>
void minimize(std::vector<blk_ref> &next_closed, std::vector<blk_ref> &closed, const grid_t<T> &values)
{
	auto opt = std::numeric_limits<rep_t>::max();
	for (auto b : closed)
	{
		const auto v = *values(b.x(), b.y());
		if (v < opt)
			opt = v;
	}
	for (auto b : closed)
	{
		const auto v = *values(b.x(), b.y());
		if (v <= opt)
			next_closed.push_back(b);
	}
}

template <typename T>
void maximize(std::vector<blk_ref> &next_closed, std::vector<blk_ref> &closed, const grid_t<T> &values)
{
	auto opt = -std::numeric_limits<rep_t>::max();
	for (auto b : closed)
	{
		const auto v = *values(b.x(), b.y());
		if (v > opt)
			opt = v;
	}
	for (auto b : closed)
	{
		const auto v = *values(b.x(), b.y());
		if (v >= opt)
			next_closed.push_back(b);
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
		two_step_logic(init);
	}

	if (basic_solver_.is_finished(actual_))
		return true;

	if (!config->strategy.heuristic_enabled)
		return false;

	do
	{
		std::vector<blk_ref> closed;
		for (auto it = actual_.begin(); it != actual_.end(); ++it)
			if (it->is_closed())
				closed.push_back(it);

		auto dist_gathered = false, safe_gathered = false;
		for (auto m : config->strategy.decision_tree)
		{
			std::vector<blk_ref> next_closed;
			switch (m)
			{
			case strategy::heuristic_method::min_mine_prob:
				gather_mine_prob();
				minimize(next_closed, closed, h_mine_prob_);
				break;
			case strategy::heuristic_method::max_zero_prob:
				if (!dist_gathered)
					gather_neighbor_dist(closed), dist_gathered = true;
				maximize(next_closed, closed, h_zero_prob_);
				break;
			case strategy::heuristic_method::max_entropy:
				if (!dist_gathered)
					gather_neighbor_dist(closed), dist_gathered = true;
				maximize(next_closed, closed, h_entropy_);
				break;
			case strategy::heuristic_method::max_zeros_prob:
				if (!safe_gathered)
					gather_safe_move(closed), safe_gathered = true;
				maximize(next_closed, closed, h_zeros_prob_);
				break;
			case strategy::heuristic_method::max_zeros_exp:
				if (!safe_gathered)
					gather_safe_move(closed), safe_gathered = true;
				maximize(next_closed, closed, h_zeros_exp_);
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

		if (b->is_mine())
			return false;

		b->set_closed(false);
		for (auto &bb : actual_)
			bb.set_front(false);
		for (auto bb : closed)
			bb->set_front(true);
		CHECK_POINT;
		two_step_logic(b);
	}
	while (!basic_solver_.is_finished(actual_));

	return true;
}

void game::two_step_logic(blk_ref b)
{
	FORCE_LOGIC(basic_solver_.try_basic_logic(b, true));

	full_solver_ = std::make_shared<full_logic>(actual_, config);
	FORCE_LOGIC(full_solver_->try_full_logics());
}

void game::gather_mine_prob()
{
	for (auto &v : h_mine_prob_)
		v = 0;

	rep_t total = 0;
	for (auto &gr : full_solver_->spec_grids_)
	{
		total += gr.second.repitition;

		auto it = gr.first.begin();
		auto itt = h_mine_prob_.begin();
		for (; it != gr.first.end(); ++it, ++itt)
		{
			if (!it->is_spec())
				continue;
			if (it->is_closed())
				*itt += gr.second.probability * gr.second.repitition;
			else if (it->is_mine())
				*itt += gr.second.repitition;
		}
	}

	for (auto &v : h_mine_prob_)
		v /= total;
}

void game::gather_neighbor_dist(const std::vector<blk_ref> &refs)
{
	for (auto &dist : h_neighbor_dist_)
		dist.fill(0);

	for (auto b : refs)
	{
		auto &dist = *h_neighbor_dist_(b.x(), b.y());
		dist.fill(0);

		rep_t total = 0;
		for (auto &gr : full_solver_->spec_grids_)
		{
			auto bg = gr.first(b.x(), b.y());
			if (bg->is_mine())
				continue;

			total += gr.second.repitition;

			size_t cnt = 0, cntm = 0;
			for (auto bb : bg.neighbors())
				if (bb->is_closed())
					cnt++;
				else if (bb->is_mine())
					cntm++;

			for (size_t i = 0; i <= cnt && i <= gr.second.total_mines; i++)
			{
				const auto d = binomial(cnt, i)
					* binomial(gr.second.closed - cnt, gr.second.total_mines - i)
					/ gr.second.closed_rep;
				dist[i + cntm] += gr.second.repitition * d;
			}
		}

		for (auto &v : dist)
			v /= total;

		*h_zero_prob_(b.x(), b.y()) = dist[0];

		auto &entropy = *h_entropy_(b.x(), b.y());
		for (auto &v : dist)
			if (v != 0)
				entropy += v * std::log(v);
	}
}

void game::gather_safe_move(const std::vector<blk_ref> &refs)
{
	throw std::runtime_error("Internal error: not implemented yet");
	for (auto b : refs)
	{
		auto &prob = *h_zeros_prob_(b.x(), b.y());
		auto &exp = *h_zeros_exp_(b.x(), b.y());

		prob = 0;
		exp = 0;

		rep_t grand_total = 0;
		for (size_t n = 0; n <= 8; n++)
		{
			// TODO
		}

		prob /= grand_total;
		exp /= grand_total;
	}
}

void game::initialize_mine(blk_ref b)
{
	b->set_mine(true);
	for (auto &n : b.neighbors())
		if (!n->is_mine())
			++*n;
}
