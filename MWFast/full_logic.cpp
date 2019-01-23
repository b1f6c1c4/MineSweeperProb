#include "full_logic.h"
#include "binomial.h"

full_logic::full_logic(grid_t<blk_t> &grid, std::shared_ptr<logic_config> config)
	: basic_logic(config), grid_(grid) { }

logic_result full_logic::try_full_logics(const blk_ref pivot, const bool force)
{
	auto flag = false;
	LOGIC(try_basic_logic(pivot, true));
	LOGIC(try_full_logics(force));
	return flag ? logic_result::dirty : logic_result::clean;
}

logic_result full_logic::try_full_logic(bool force)
{
	if (!force && !(config->strategy.logic & strategy_t::logic_method::full))
		return logic_result::clean;

	prepare_full_logic();
	if (front_set_.empty())
	{
		const auto st = get_stats(grid_);
		auto prob = rep_t(st.rest_mines);
		prob /= st.closed;
		spec_grids_.emplace_back(std::make_pair(grid_, spec_stat{
			1,
			prob,
			1,
			st.closed,
			st.rest_mines
			}));
		return logic_result::clean;
	}

	auto grid(grid_);
	for (auto &bb : grid)
		if (bb.is_closed())
			bb.set_spec(true), bb.set_mine(false), bb.set_neighbor(0);

	speculative_fork(fork_directive{ 0, &grid, false });
	speculative_fork(fork_directive{ 0, &grid, true });

	if (spec_grids_.empty())
		return logic_result::invalid;

	auto flag = false;

	grid_t<uint8_t> tmp(grid_.width(), grid_.height(), 0x00);
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
		auto it = grid_.begin();
		auto itt = tmp.begin();
		for (; it != grid_.end(); ++it, ++itt)
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

logic_result full_logic::try_full_logics(bool force)
{
	bool flag;
	do
	{
		flag = false;
		LOGIC(try_basic_logics(grid_));
		LOGIC(try_full_logic(force));
	} while (flag);
	return logic_result::clean;
}

const grid_t<blk_t> & full_logic::actual() const
{
	return grid_;
}

const std::vector<spec_grid_t> & full_logic::spec() const
{
	return spec_grids_;
}

std::shared_ptr<logic_config> full_logic::get_config() const
{
	return config;
}

void full_logic::prepare_full_logic()
{
	front_set_.clear();
	spec_grids_.clear();
	for (auto &b : grid_)
		b.set_front(false);

	for (auto it = grid_.begin(); it != grid_.end(); ++it)
		if (!it->is_closed() && !it->is_mine())
			for (auto b : it.neighbors())
				if (b->is_closed() && !b->is_front())
					b->set_front(true), front_set_.emplace_back(b);
}

void full_logic::speculative_fork(const fork_directive directive)
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
			speculative_fork(fork_directive{ id, &grid, false });
			speculative_fork(fork_directive{ id, &grid, true });
			return;
		}
	}

	if (!config->is_fixed_mines)
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
