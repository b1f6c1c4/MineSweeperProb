#include "heuristic.h"
#include "binomial.h"
#include <cmath>

//#define EXTRA_VERBOSE

#ifdef NDEBUG
#undef EXTRA_VERBOSE
#endif

heuristic_solver::heuristic_solver(const full_logic &logic)
	: logic_(logic),
	  gathered_mine_prob_est_(false),
	  h_mine_prob_est_(logic.actual().width(), logic.actual().height(), 0),
	  gathered_zeros_est_(false),
	  h_zero_prob_est_(logic.actual().width(), logic.actual().height(), 0),
	  h_zeros_prob_est_(logic.actual().width(), logic.actual().height(), 0),
	  h_zeros_exp_est_(logic.actual().width(), logic.actual().height(), 0),
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

void heuristic_solver::filter_lp(blk_refs &next_closed, const blk_refs &closed)
{
	gather_mine_prob_est();
	minimize(next_closed, closed, h_mine_prob_est_);
}

void heuristic_solver::filter_ls(blk_refs &next_closed, const blk_refs &closed)
{
	gather_zeros_est(closed);
	maximize(next_closed, closed, h_zeros_prob_est_);
}

void heuristic_solver::filter_le(blk_refs &next_closed, const blk_refs &closed)
{
	gather_zeros_est(closed);
	maximize(next_closed, closed, h_zeros_exp_est_);
}

void heuristic_solver::filter_lz(blk_refs &next_closed, const blk_refs &closed)
{
	gather_zeros_est(closed);
	maximize(next_closed, closed, h_zero_prob_est_);
}

void heuristic_solver::gather_mine_prob_est()
{
	if (gathered_mine_prob_est_)
		return;
	gathered_mine_prob_est_ = true;

	auto &lp = logic_.lp_solver();
	auto it = lp.get_result().begin();
	auto ait = logic_.areas().begin();
	for (; it != lp.get_result().end(); ++it, ++ait)
	{
		auto prob = rep_t(it->second);
		prob /= ait->size();

		for (auto b : *ait)
			*h_mine_prob_est_(b.x(), b.y()) += prob;
	}
}

void heuristic_solver::gather_zeros_est(const blk_refs &refs)
{
	if (gathered_zeros_est_)
		return;
	gathered_zeros_est_ = true;

#ifndef NDEBUG
	std::cerr << "Calculating sz on" << std::endl;
	for (auto ref : refs)
		ref->set_front(true);
	std::cerr << logic_.actual();
	for (auto ref : refs)
		ref->set_front(false);
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
		size_t lb = 0, ub = 0;
		for (auto bb : b.neighbors())
			if (bb->is_closed())
				ub++;
			else if (bb->is_mine())
				lb++, ub++;

		auto &z_est = *h_zero_prob_est_(b.x(), b.y());
		auto &p_est = *h_zeros_prob_est_(b.x(), b.y());
		auto &e_est = *h_zeros_exp_est_(b.x(), b.y());

		z_est = 0;
		p_est = 0;
		e_est = 0;

		rep_t total = 0;
		auto logic = logic_.logic_fork_spec(b, static_cast<uint8_t>(lb));
		for (auto n = lb; n <= ub; n++)
		{
			logic.modify_spec(b, static_cast<uint8_t>(n));
			if (logic.try_full_logics(true) == logic_result::invalid)
				continue;

			rep_t cnt = 1;
			auto it = logic.areas().begin();
			auto itt = logic.lp_solver().get_result().begin();
			for (; it != logic.areas().end(); ++it, ++itt)
			{
				cnt *= (binomial(it->size(), itt->first) + binomial(it->size(), itt->second)) / 2;
				cnt *= itt->second - itt->first + 1;
			}

			if (n == lb)
				z_est += cnt;

			const auto safe = logic.safe_count();

#ifdef EXTRA_VERBOSE
			std::cerr << "total * p(g_" << b.x() << b.y() << "," << n << ")=" << cnt;
			std::cerr << " safe=" << safe;
			std::cerr << std::endl;
#endif

			total += cnt;

			if (safe != 0)
				p_est += cnt;
			e_est += safe * cnt;
		}

		z_est /= total;
		p_est /= total;
		e_est /= total;

#ifdef EXTRA_VERBOSE
		std::cerr << "g_" << b.x() << b.y() << " lb=" << lb << " ub=" << ub << std::endl;
		std::cerr << "total = " << total << std::endl;
		std::cerr << "zero = " << z_est << " prob = " << p_est << " exp = " << e_est << std::endl << std::endl;
#endif
	}
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
	for (auto ref : refs)
		ref->set_front(true);
	std::cerr << logic_.actual();
	for (auto ref : refs)
		ref->set_front(false);
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
		size_t lb = 0, ub = 0;
		for (auto bb : b.neighbors())
			if (bb->is_closed())
				ub++;
			else if (bb->is_mine())
				lb++, ub++;

		auto &prob = *h_zeros_prob_(b.x(), b.y());
		auto &exp = *h_zeros_exp_(b.x(), b.y());

		prob = 0;
		exp = 0;

		rep_t total = 0;
		auto logic = logic_.logic_fork_spec(b, static_cast<uint8_t>(lb));
		for (auto n = lb; n <= ub; n++)
		{
			logic.modify_spec(b, static_cast<uint8_t>(n));
			if (logic.try_full_logics(true) == logic_result::invalid)
				continue;

			const auto cnt = logic.rep_count();
			const auto safe = logic.safe_count();

#ifdef EXTRA_VERBOSE
			std::cerr << "total * p(g_" << b.x() << b.y() << "," << n << ")=" << cnt;
			std::cerr << " safe=" << safe;
			std::cerr << std::endl;
#endif

			total += cnt;

			if (safe != 0)
				prob += cnt;
			exp += cnt * safe;
		}

		if (total == 0)
			throw std::runtime_error("Internal error: distribution doesn't exist");

		prob /= total;
		exp /= total;

#ifdef EXTRA_VERBOSE
		std::cerr << "g_" << b.x() << b.y() << " lb=" << lb << " ub=" << ub << std::endl;
		std::cerr << "total = " << total << std::endl;
		std::cerr << "prob = " << prob << " exp = " << exp << std::endl << std::endl;
#endif
	}
}
