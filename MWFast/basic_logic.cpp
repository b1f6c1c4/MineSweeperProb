#include "basic_logic.h"

logic_config::logic_config(const bool t, const size_t m, const double p)
	: strategy("FL@[1,1]-Pure"), is_fixed_mines(t), total_mines(m), probability(p) { }

logic_config::logic_config(strategy_t &&s, const bool t, const size_t m, const double p)
	: strategy(s), is_fixed_mines(t), total_mines(m), probability(p) { }

basic_logic::basic_logic(std::shared_ptr<logic_config> config) : config(std::move(config)) {}

logic_result basic_logic::try_basic_logic(blk_ref b, const bool aggressive) const
{
	if (b->is_closed())
		throw std::runtime_error("Internal error: try to logic a closed block");

	auto flag = false;

	if (config->strategy.logic & strategy_t::logic_method::passive)
	{
		if (!b->is_spec() && !b->is_mine() && b->neighbor() == 0)
			for (auto bb : b.neighbors())
				if (bb->is_closed())
				{
					if (!b->is_spec() && bb->is_mine())
						return logic_result::invalid;
					bb->set_closed(false);
					flag = true;
					LOGIC(try_basic_logic(bb, aggressive));
				}
	}

	if (config->strategy.logic & strategy_t::logic_method::single)
	{
		if (!b->is_spec() && !b->is_mine())
			LOGIC(try_single_logic(b, aggressive));

		else if (aggressive)
			for (const auto bb : b.neighbors())
				if (!bb->is_closed() && !bb->is_mine())
					LOGIC(try_single_logic(bb, false));
	}

	return flag ? logic_result::dirty : logic_result::clean;
}

logic_result basic_logic::try_basic_logics(grid_t<blk_t> &grid) const
{
	bool flag;
	do
	{
		flag = false;
		for (auto it = grid.begin(); it != grid.end(); ++it)
			if (!it->is_closed())
				LOGIC(try_basic_logic(it, false));

		LOGIC(try_ext_logic(grid));

		if (!flag && (config->strategy.logic & strategy_t::logic_method::dual))
			for (auto it = grid.begin(); it != grid.end(); ++it)
				if (!it->is_closed() && !it->is_mine())
					for (auto b2 : it.neighbors())
						if (!b2->is_closed() && !b2->is_mine())
							LOGIC(try_dual_logic(it, b2));
	}
	while (flag);
	return logic_result::clean;
}

logic_result basic_logic::try_single_logic(blk_ref b, const bool aggressive) const
{
	if (b->is_closed())
		throw std::runtime_error("Internal error: try single logic on a closed");

	if (b->is_spec())
		return logic_result::clean;

	if (b->is_mine())
		throw std::runtime_error("Internal error: try single logic on a mine");

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
				LOGIC(try_basic_logic(bb, aggressive));
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
				LOGIC(try_basic_logic(bb, aggressive));
			}
	}

	return flag ? logic_result::dirty : logic_result::clean;
}

logic_result refs_unit(blk_refs &refs, const size_t lb, const size_t ub)
{
	auto flag = false;
	if (lb == refs.size())
		for (auto b : refs)
		{
			if (!b->is_spec() && !b->is_mine())
				return logic_result::invalid;
			b->set_closed(false);
			flag = true;
		}
	else if (ub == 0)
		for (auto b : refs)
		{
			if (!b->is_spec() && b->is_mine())
				return logic_result::invalid;
			b->set_closed(false);
			flag = true;
		}

	return flag ? logic_result::dirty : logic_result::clean;
}

logic_result basic_logic::try_dual_logic(blk_ref b1, blk_ref b2) const
{
	if (!(config->strategy.logic & strategy_t::logic_method::dual))
		return logic_result::clean;

	if (b1->is_closed() || b2->is_closed())
		throw std::runtime_error("Internal error: try dual logic on a closed");

	if (b1->is_spec() || b2->is_spec())
		return logic_result::clean;

	if (b1->is_mine() || b2->is_mine())
		throw std::runtime_error("Internal error: try dual logic on a mine");

	size_t cntm_l = b1->neighbor(), cntm_r = b2->neighbor();

	if (cntm_l == 0 || cntm_r == 0) // Should have been handled by single
		return logic_result::clean;

	blk_refs left, middle, right;
	for (const auto bl : b1.neighbors())
		if (bl->is_closed())
			for (const auto br : b2.neighbors())
				if (bl == br)
				{
					middle.push_back(bl);
					break;
				}

	for (const auto b : b1.neighbors())
	{
		if (!b->is_closed() && b->is_mine())
			cntm_l--;
		if (b->is_closed() && std::find(middle.begin(), middle.end(), b) == middle.end())
			left.push_back(b);
	}
	for (const auto b : b2.neighbors())
	{
		if (!b->is_closed() && b->is_mine())
			cntm_r--;
		if (b->is_closed() && std::find(middle.begin(), middle.end(), b) == middle.end())
			right.push_back(b);
	}

	if (cntm_l == 0 || cntm_r == 0) // Should have been handled by single
		return logic_result::clean;

	size_t l_lb = 0, l_ub = left.size();
	size_t m_lb = 0, m_ub = middle.size();
	size_t r_lb = 0, r_ub = right.size();

	if (cntm_l > l_ub + m_lb)
		m_lb = cntm_l - l_ub;
	if (cntm_l < l_lb + m_ub)
		m_ub = cntm_l - l_lb;
	if (cntm_r > r_ub + m_lb)
		m_lb = cntm_r - r_ub;
	if (cntm_r < r_lb + m_ub)
		m_ub = cntm_r - r_lb;

	if (cntm_l > l_lb + m_ub)
		l_lb = cntm_l - m_ub;
	if (cntm_l < l_ub + m_lb)
		l_ub = cntm_l - m_lb;
	if (cntm_r > r_lb + m_ub)
		r_lb = cntm_r - m_ub;
	if (cntm_r < r_ub + m_lb)
		r_ub = cntm_r - m_lb;

	auto flag = false;
	LOGIC(refs_unit(left, l_lb, l_ub));
	LOGIC(refs_unit(middle, m_lb, m_ub));
	LOGIC(refs_unit(right, r_lb, r_ub));

	return flag ? logic_result::dirty : logic_result::clean;
}

logic_result basic_logic::try_ext_logic(grid_t<blk_t> &grid) const
{
	if (!(config->strategy.logic & strategy_t::logic_method::extended))
		return logic_result::clean;
	if (!config->is_fixed_mines)
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

stats basic_logic::get_stats(const grid_t<blk_t> &grid) const
{
	stats st{0, config->total_mines};
	for (auto &b : grid)
		if (b.is_closed())
			st.closed++;
		else if (b.is_mine())
			st.rest_mines--;
	return st;
}

bool basic_logic::is_finished(grid_t<blk_t> &grid) const
{
	for (const auto &b : grid)
		if (b.is_closed())
			return false;

	return true;
}
