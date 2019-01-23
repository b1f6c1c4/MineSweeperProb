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
	: strategy(st), is_fixed_mines(false), total_mines(-1), actual_(w, h, blk_t::closed_simple(0)),
	  h_mine_prob_(w, h, 0), h_neighbor_dist_(w, h, std::array<rep_t, 8>{}),
	  h_zero_prob_(w, h, 0), h_entropy_(w, h, 0),
	  h_zeros_prob_(w, h, 0), h_zeros_exp_(w, h, 0) { }

const grid_t<blk_t> &game::grid() const
{
	return actual_;
}

void game::fill_fixed(size_t m, const bool buff)
{
	is_fixed_mines = true;
	total_mines = m;

	const std::uniform_int_distribution<size_t> dx(0, actual_.width() - 1);
	const std::uniform_int_distribution<size_t> dy(0, actual_.height() - 1);

	while (m)
	{
		const auto x = dx(device);
		const auto y = dy(device);

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
		--m;
	}
}

void game::fill_prob(const double p, const bool buff)
{
	is_fixed_mines = true;
	total_mines = -1;

	const std::bernoulli_distribution d(p);

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

#define FORCE_LOGIC(X) if ((X) == logic_result::invalid) \
throw std::runtime_error("Internal error: invalid actual grid")

bool game::run()
{
	auto init = actual_(strategy.initial_x, strategy.initial_y);
	if (init->is_closed())
	{
		if (!init->is_closed())
			throw std::runtime_error("Internal error: try to open a opened block");

		if (init->is_mine())
			throw std::runtime_error("Internal error: try to logically open a mine");

		init->set_closed(false);
		FORCE_LOGIC(try_basic_logic(init, true));
		FORCE_LOGIC(try_full_logics());
	}

	if (is_finished(actual_))
		return true;

	if (!strategy.heuristic_enabled)
		return false;

	do
	{
		std::vector<blk_ref> closed;
		for (auto it = actual_.begin(); it != actual_.end(); ++it)
			if (it->is_closed())
				closed.push_back(it);

		auto dist_gathered = false, safe_gathered = false;
		for (auto m : strategy.decision_tree)
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
		FORCE_LOGIC(try_basic_logic(b, true));
		FORCE_LOGIC(try_full_logics());
	}
	while (!is_finished(actual_));

	return true;
}

bool game::is_finished(grid_t<blk_t> &grid) const
{
	for (const auto &b : grid)
		if (b.is_closed())
			return false;

	return true;
}

#define LOGIC(X) do { switch (X) \
{ \
case logic_result::clean: break; \
case logic_result::dirty: flag = true; break; \
case logic_result::invalid: return logic_result::invalid; \
default: throw std::runtime_error("Internal error: unknown logic result"); \
} } while (false)

game::logic_result game::try_basic_logic(blk_ref b, const bool aggressive)
{
	if (b->is_closed())
		throw std::runtime_error("Internal error: try to logic a closed block");

	auto flag = false;

	if (strategy.logic & strategy::logic_method::passive)
	{
		if (!b->is_mine() && !b->is_spec() && b->neighbor() == 0)
			for (auto bb : b.neighbors())
				if (bb->is_closed())
				{
					if (bb->is_mine())
						return logic_result::invalid;
					bb->set_closed(false);
					flag = true;
					if (try_basic_logic(bb, aggressive) == logic_result::invalid)
						return logic_result::invalid;
				}
	}

	if (strategy.logic & strategy::logic_method::single)
	{
		if (!b->is_mine())
			LOGIC(try_single_logic(b, aggressive));

		else if (aggressive)
			for (const auto bb : b.neighbors())
				if (!bb->is_mine())
					LOGIC(try_single_logic(bb, false));
	}

	return flag ? logic_result::dirty : logic_result::clean;
}

game::logic_result game::try_basic_logics(grid_t<blk_t> &grid)
{
	bool flag;
	do
	{
		flag = false;
		for (auto it = grid.begin(); it != grid.end(); ++it)
			if (!it->is_closed())
				LOGIC(try_basic_logic(it, false));

		LOGIC(try_ext_logic(grid));
	}
	while (flag);
	return logic_result::clean;
}

game::logic_result game::try_single_logic(blk_ref b, const bool aggressive)
{
	if (b->is_mine())
		throw std::runtime_error("Internal error: try single logic on a mine");

	if (b->is_spec())
		return logic_result::clean;

	auto flag = false;

	size_t cnt = 0, cntm = 0;
	for (const auto bb : b.neighbors())
	{
		if (bb->is_closed())
			cnt++;
		else if (bb->is_mine())
			cntm++;
	}
	if (b->neighbor() < cntm)
		return logic_result::invalid;
	if (b->neighbor() > cntm + cnt)
		return logic_result::invalid;

	if (b->neighbor() == cntm)
	{
		for (auto bb : b.neighbors())
			if (bb->is_closed())
			{
				if (!bb->is_spec() && bb->is_mine())
					return logic_result::invalid;
				bb->set_closed(false);
				bb->set_mine(false);
				flag = true;
				try_basic_logic(bb, aggressive);
			}
	}
	else if (b->neighbor() == cntm + cnt)
	{
		for (auto bb : b.neighbors())
			if (bb->is_closed())
			{
				if (!bb->is_spec() && !bb->is_mine())
					return logic_result::invalid;
				bb->set_closed(false);
				bb->set_mine(true);
				flag = true;
				try_basic_logic(bb, aggressive);
			}
	}

	return flag ? logic_result::dirty : logic_result::clean;
}

game::logic_result game::try_ext_logic(grid_t<blk_t> &grid)
{
	if (!(strategy.logic & strategy::logic_method::extended))
		return logic_result::clean;
	if (!is_fixed_mines)
		return logic_result::clean;

	const auto st = get_stats(grid);

	if (st.rest_mines > grid.width() * grid.height())
		return logic_result::invalid;

	if (st.rest_mines > st.closed)
		return logic_result::invalid;

	if (st.rest_mines == 0)
	{
		auto flag = false;

		for (auto &bb : grid)
			if (bb.is_closed())
			{
				if (!bb.is_spec() && bb.is_mine())
					return logic_result::invalid;
				bb.set_closed(false);
				bb.set_mine(false);
				flag = true;
			}

		return flag ? logic_result::dirty : logic_result::clean;
	}

	if (st.rest_mines == st.closed)
	{
		auto flag = false;

		for (auto &bb : grid)
			if (bb.is_closed())
			{
				if (!bb.is_spec() && !bb.is_mine())
					return logic_result::invalid;
				bb.set_closed(false);
				bb.set_mine(true);
				flag = true;
			}

		return flag ? logic_result::dirty : logic_result::clean;
	}

	return logic_result::clean;
}

game::logic_result game::try_full_logic()
{
	if (!(strategy.logic & strategy::logic_method::full))
		return logic_result::clean;

	prepare_full_logic();
	if (front_set_.empty())
	{
		const auto st = get_stats(actual_);
		auto prob = rep_t(st.rest_mines);
		prob /= st.closed;
		spec_grids_.emplace_back(std::make_pair(actual_, spec_stat{
			1,
			prob,
			1,
			st.closed,
			st.rest_mines
		}));
		return logic_result::clean;
	}

	auto grid(actual_);
	for (auto &bb : grid)
		if (bb.is_closed())
			bb.set_spec(true), bb.set_mine(false), bb.set_neighbor(0);

	speculative_fork(fork_directive{0, &grid, false});
	speculative_fork(fork_directive{0, &grid, true});

	if (spec_grids_.empty())
		return logic_result::invalid;

	auto flag = false;

	grid_t<uint8_t> tmp(actual_.width(), actual_.height(), 0x00);
	for (auto &gr : spec_grids_)
	{
		auto it = gr.first.begin();
		auto itt = tmp.begin();
		for (; it != gr.first.end(); ++it, ++itt)
		{
			if (it->is_closed())
				*itt = 0xff;
			else if (it->is_mine())
				*itt |= 0xf0;
			else
				*itt |= 0x0f;
		}
	}
	{
		auto it = actual_.begin();
		auto itt = tmp.begin();
		for (; it != actual_.end(); ++it, ++itt)
		{
			if (!it->is_closed())
				continue;

			if (!(*itt & 0xf0))
			{
				if (it->is_mine())
					return logic_result::invalid;
				it->set_closed(false);
				flag |= true;
			}

			if (!(*itt & 0x0f))
			{
				if (!it->is_mine())
					return logic_result::invalid;
				it->set_closed(false);
				flag |= true;
			}
		}

		return flag ? logic_result::dirty : logic_result::clean;
	}
}

game::logic_result game::try_full_logics()
{
	bool flag;
	do
	{
		flag = false;
		LOGIC(try_basic_logics(actual_));
		LOGIC(try_full_logic());
	}
	while (flag);
	return logic_result::clean;
}

game::stats game::get_stats(const grid_t<blk_t> &grid) const
{
	stats st{0, total_mines};
	for (auto &b : grid)
		if (b.is_closed())
			st.closed++;
		else if (b.is_mine())
			st.rest_mines--;
	return st;
}

void game::prepare_full_logic()
{
	front_set_.clear();
	spec_grids_.clear();
	for (auto &b : actual_)
		b.set_front(false);

	for (auto it = actual_.begin(); it != actual_.end(); ++it)
		if (!it->is_closed() && !it->is_mine())
			for (auto b : it.neighbors())
				if (b->is_closed() && !b->is_front())
					b->set_front(true), front_set_.emplace_back(b);
}

void game::speculative_fork(const fork_directive directive)
{
	auto grid(*directive.base);
	auto it = front_set_[directive.index];
	auto b = grid(it.x(), it.y());
	if (!b->is_closed())
		throw std::runtime_error("Internal error: try to spec open a opened block");

	b->set_closed(false);
	b->set_mine(directive.value);

	if (try_basic_logic(b, true) == logic_result::invalid)
		return;
	if (try_basic_logics(grid) == logic_result::invalid)
		return;

	if (!is_finished(grid))
	{
		size_t id;
		for (id = directive.index + 1; id < front_set_.size(); id++)
		{
			auto nit = front_set_[id];
			auto nb = grid(nit.x(), nit.y());
			if (nb->is_closed())
				break;
		}
		if (id < front_set_.size())
		{
			speculative_fork(fork_directive{id, &grid, false});
			speculative_fork(fork_directive{id, &grid, true});
			return;
		}
	}

	if (!is_fixed_mines)
		throw std::runtime_error("P minesweeper is NOT yet supported.");

	const auto st = get_stats(grid);
	const auto rep = binomial(st.closed, st.rest_mines);
	auto prob = rep_t(st.rest_mines);
	prob /= st.closed;
	spec_grids_.emplace_back(std::make_pair(grid, spec_stat{
		rep,
		prob,
		rep,
		st.closed,
		st.rest_mines
	}));
}

void game::gather_mine_prob()
{
	for (auto &v : h_mine_prob_)
		v = 0;

	rep_t total = 0;
	for (auto &gr : spec_grids_)
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
		for (auto &gr : spec_grids_)
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
	for (auto b : refs)
	{
		auto &prob = *h_zeros_prob_(b.x(), b.y());
		auto &exp = *h_zeros_exp_(b.x(), b.y());

		prob = 0;
		exp = 0;

		rep_t grand_total = 0;
		for (size_t n = 0; n <= 8; n++)
		{
			grid_t<uint8_t> tmp(actual_.width(), actual_.height(), 0x00);

			rep_t total = 0;
			for (auto &gr : spec_grids_)
			{
				auto bg = gr.first(b.x(), b.y());
				if (bg->is_mine())
					continue;

				size_t cnt = 0, cntm = 0;
				for (auto bb : bg.neighbors())
					if (bb->is_closed())
						cnt++;
					else if (bb->is_mine())
						cntm++;

				if (cntm > n || n > gr.second.total_mines + cntm)
					continue;

				const auto i = n - cntm;
				const auto d = binomial(cnt, i)
					* binomial(gr.second.closed - cnt, gr.second.total_mines - i)
					/ gr.second.closed_rep;
				const auto p = gr.second.repitition * d;
				if (p == 0)
					continue;

				total += p;

				auto it = gr.first.begin();
				auto itt = tmp.begin();
				for (; it != gr.first.end(); ++it, ++itt)
					if (!it->is_spec())
						*itt = 0xff;
					else if (it.x() == b.x() && it.y() == b.y())
						*itt = 0xff;
					else if (it->is_closed())
						*itt |= gr.second.total_mines ? 0x00 : 0xff;
					else if (it->is_mine())
						*itt = 0xff;
			}

			size_t safe = 0;
			for (auto &bb : tmp)
				if (!bb)
					safe++;

			prob += safe ? total : 0;
			exp += safe * total;

			grand_total += total;
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
