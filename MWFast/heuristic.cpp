#include "heuristic.h"
#include "binomial.h"
#include <cmath>

heuristic_solver::heuristic_solver(const full_logic &logic)
	: logic_(logic),
	  gathered_mine_prob_(false),
	  h_mine_prob_(logic.actual().width(), logic.actual().height(), 0),
	  gathered_neighbor_dist(false),
	  h_neighbor_dist_(logic.actual().width(), logic.actual().height(), std::array<rep_t, 8>{}),
	  h_zero_prob_(logic.actual().width(), logic.actual().height(), 0),
	  h_entropy_(logic.actual().width(), logic.actual().height(), 0),
	  gathered_safe_move(false),
	  h_zeros_prob_(logic.actual().width(), logic.actual().height(), 0),
	  h_zeros_exp_(logic.actual().width(), logic.actual().height(), 0) { }

template <typename T>
void minimize(blk_refs &next_closed, const blk_refs &closed, const grid_t<T> &values)
{
	auto opt = std::numeric_limits<rep_t>::max();
	for (const auto b : closed)
	{
		const auto v = *values(b.x(), b.y());
		if (v < opt)
			opt = v;
	}
	for (const auto b : closed)
	{
		const auto v = *values(b.x(), b.y());
		if (v <= opt)
			next_closed.push_back(b);
	}
}

template <typename T>
void maximize(blk_refs &next_closed, const blk_refs &closed, const grid_t<T> &values)
{
	auto opt = -std::numeric_limits<rep_t>::max();
	for (const auto b : closed)
	{
		const auto v = *values(b.x(), b.y());
		if (v > opt)
			opt = v;
	}
	for (const auto b : closed)
	{
		const auto v = *values(b.x(), b.y());
		if (v >= opt)
			next_closed.push_back(b);
	}
}

void heuristic_solver::filter_p(blk_refs &next_closed, const blk_refs &closed)
{
	gather_mine_prob();
	minimize(next_closed, closed, h_mine_prob_);
}

void heuristic_solver::filter_s(blk_refs &next_closed, const blk_refs &closed)
{
	gather_safe_move(closed);
	maximize(next_closed, closed, h_zeros_prob_);
}

void heuristic_solver::filter_e(blk_refs &next_closed, const blk_refs &closed)
{
	gather_safe_move(closed);
	maximize(next_closed, closed, h_zeros_exp_);
}

void heuristic_solver::filter_q(blk_refs &next_closed, const blk_refs &closed)
{
	gather_neighbor_dist(closed);
	maximize(next_closed, closed, h_entropy_);
}

void heuristic_solver::filter_z(blk_refs &next_closed, const blk_refs &closed)
{
	gather_neighbor_dist(closed);
	maximize(next_closed, closed, h_zero_prob_);
}

void heuristic_solver::gather_mine_prob()
{
	if (gathered_mine_prob_)
		return;
	gathered_mine_prob_ = true;

	for (auto &v : h_mine_prob_)
		v = 0;

	rep_t total = 0;
	for (auto &gr : logic_.spec())
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

void heuristic_solver::gather_neighbor_dist(const blk_refs &refs)
{
	if (gathered_neighbor_dist)
		return;
	gathered_neighbor_dist = true;

	for (auto &dist : h_neighbor_dist_)
		dist.fill(0);

	for (auto b : refs)
	{
		auto &dist = *h_neighbor_dist_(b.x(), b.y());
		dist.fill(0);

		rep_t total = 0;
		for (auto &gr : logic_.spec())
		{
			auto bg = gr.first(b.x(), b.y());
			if (bg->is_mine())
				continue;

			total += gr.second.repitition;

			size_t cnt = 0, cntm = 0;
			for (const auto bb : bg.neighbors())
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

void heuristic_solver::gather_safe_move(const blk_refs &refs)
{
	if (gathered_safe_move)
		return;
	gathered_safe_move = true;

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
