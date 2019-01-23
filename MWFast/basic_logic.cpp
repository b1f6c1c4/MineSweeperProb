#include "basic_logic.h"

logic_config::logic_config(const bool t, const size_t m)
	: strategy("FL@[1,1]-NH"), is_fixed_mines(t), total_mines(m) { }

logic_config::logic_config(strategy_t &&s, const bool t, const size_t m)
	: strategy(s), is_fixed_mines(t), total_mines(m) { }

basic_logic::basic_logic(std::shared_ptr<logic_config> config) : config(std::move(config)) {}

logic_result basic_logic::try_basic_logic(blk_ref b, const bool aggressive) const
{
	if (b->is_closed())
		throw std::runtime_error("Internal error: try to logic a closed block");

	auto flag = false;

	if (config->strategy.logic & strategy_t::logic_method::passive)
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

	if (config->strategy.logic & strategy_t::logic_method::single)
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
	}
	while (flag);
	return logic_result::clean;
}

logic_result basic_logic::try_single_logic(blk_ref b, const bool aggressive) const
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
