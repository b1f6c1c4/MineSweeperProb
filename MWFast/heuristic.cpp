#include "heuristic.h"
#include "binomial.h"
#include <cmath>

#define EXTRA_VERBOSE

heuristic_solver::heuristic_solver(const full_logic &logic)
	: logic_(logic),
	  gathered_mine_prob_(false),
	  h_mine_prob_(logic.actual().width(), logic.actual().height(), 0),
	  gathered_neighbor_dist(false),
	  h_neighbor_dist_(logic.actual().width(), logic.actual().height(), std::array<rep_t, 9>{}),
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
	for (auto &gr : logic_.specs())
	{
		total += gr.second.repitition;

		auto it = gr.first.begin();
		auto ait = logic_.areas().begin();
		for (; it != gr.first.end(); ++it, ++ait)
		{
			auto prob = rep_t(*it);
			prob /= ait->size();

			for (auto b : *ait)
				*h_mine_prob_(b.x(), b.y()) += prob * gr.second.repitition;
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
	/*
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

			size_t extra = 0;
			rep_t lambda = 1;
			if (bg->is_closed())
			{
				extra = 1;
				lambda = rep_t(gr.second.total_mines);
				lambda /= gr.second.closed;
				lambda = 1 - lambda;
			}

			total += gr.second.repitition * lambda;

			size_t cnt = 0, cntm = 0;
			for (const auto bb : bg.neighbors())
				if (bb->is_closed())
					cnt++;
				else if (bb->is_mine())
					cntm++;
			for (size_t i = 0; i <= cnt && i <= gr.second.total_mines; i++)
			{
				if (gr.second.closed < cnt + extra)
					throw std::runtime_error("Internal error: no enough closed blocks");
				if (gr.second.total_mines < i)
					throw std::runtime_error("Internal error: no enough total mines");

				const auto d = binomial(cnt, i)
					* binomial(gr.second.closed - cnt - extra, gr.second.total_mines - i)
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
				entropy -= v * std::log(v);

#ifdef EXTRA_VERBOSE
		for (size_t i = 0; i <= 8; i++)
			std::cerr << "p(g_" << b.x() << "," << i << ")=" << dist[i] << std::endl;
		std::cerr << "total = " << total << std::endl << std::endl;
		std::cerr << "entropy = " << entropy << " zero = " << dist[0] << std::endl << std::endl;
#endif
	}*/
}

void heuristic_solver::gather_safe_move(const blk_refs &refs)
{
	if (gathered_safe_move)
		return;
	gathered_safe_move = true;

#ifndef NDEBUG
	std::cerr << "Calculating SE on" << std::endl;
	std::cerr << logic_.actual();
#endif

	grid_t<uint8_t> btmp(logic_.actual().width(), logic_.actual().height(), 0xff);
	{
		auto it = logic_.actual().begin();
		auto itt = btmp.begin();
		for (; it != logic_.actual().end(); ++it, ++itt)
			if (it->is_closed())
				*itt = 0x00;
	}

	for (auto b : refs)
	{
		auto &prob = *h_zeros_prob_(b.x(), b.y());
		auto &exp = *h_zeros_exp_(b.x(), b.y());

		prob = 0;
		exp = 0;

		rep_t total = 0;
		for (uint8_t n = 0; n <= 8; n++)
		{
			auto logic = logic_.fork(b, n);
			if (logic.try_full_logics(true) == logic_result::invalid)
				continue;

			rep_t cnt = 0;
			auto tmp(btmp);
			*tmp(b.x(), b.y()) = 0xff;

			for (auto &gr : logic.specs())
			{
				cnt += gr.second.repitition;
				auto it = logic_.actual().begin();
				auto itt = tmp.begin();
				for (; it != logic_.actual().end(); ++it, ++itt)
					if (it->is_closed())
						*itt = 0xff;
					else if (it->is_mine())
						*itt = 0xff;
			}

			size_t safe = 0;
			for (auto &bt : tmp)
				if (!bt)
					safe++;

#ifdef EXTRA_VERBOSE
			std::cerr << "total * p(g_" << b.x() << b.y() << "," << static_cast<size_t>(n) << ")=" << cnt;
			std::cerr << " safe=" << safe << " ";
			for (auto &bt : tmp)
				if (!bt)
					std::cerr << "+";
				else
					std::cerr << "-";
			std::cerr << std::endl;
#endif

			total += cnt;

			if (safe != 0)
				prob += cnt;
			exp += cnt * safe;
		}

		prob /= total;
		exp /= total;

#ifdef EXTRA_VERBOSE
		std::cerr << "total = " << total << std::endl << std::endl;
		std::cerr << "prob = " << prob << " exp = " << exp << std::endl << std::endl;
#endif
	}
}
