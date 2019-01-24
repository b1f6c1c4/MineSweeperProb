#include "full_logic.h"
#include "binomial.h"

bool blk_ref_lt::operator()(const blk_ref &lhs, const blk_ref &rhs) const
{
	return &*lhs < &*rhs;
}

conn_t::conn_t(blk_ref b): hash(0), ref(b)
{
	for (auto bb : b.neighbors())
		if (!bb->is_closed())
			if (!bb->is_mine())
				emplace(bb);

	for (auto bb : *this)
	{
		hash += reinterpret_cast<size_t>(&*bb);
		hash <<= 7;
	}

	hash++; // hash will never be 0
}

bool conn_lt::operator()(const conn_t &lhs, const conn_t &rhs) const
{
	return lhs.hash < rhs.hash;
}

full_logic::full_logic(grid_t<blk_t> &grid, std::shared_ptr<logic_config> config)
	: basic_logic(config), grid_(grid), grid_st_{}, member_(grid.width(), grid.height(), nullptr),
	  neighbors_(grid.width(), grid.height(), std::vector<area*>{}), num_areas_(0) { }

logic_result full_logic::try_full_logics(const blk_ref pivot, const bool force)
{
	auto flag = false;
	LOGIC(try_basic_logic(pivot, true));
	LOGIC(try_full_logics(force));
	return flag ? logic_result::dirty : logic_result::clean;
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

const std::list<area> & full_logic::areas() const
{
	return areas_;
}

const std::vector<spec_t> & full_logic::specs() const
{
	return spec_grids_;
}

std::shared_ptr<logic_config> full_logic::get_config() const
{
	return config;
}

logic_result full_logic::try_full_logic(const bool force)
{
	if (!force && !(config->strategy.logic & strategy_t::logic_method::full))
		return logic_result::clean;

	auto grid(grid_);
	for (auto &bb : grid)
		if (bb.is_closed())
			bb.set_spec(true), bb.set_mine(false), bb.set_neighbor(0);

	grid_st_ = get_stats(grid_);
	prepare_full_logic();

	speculative_fork(fork_directive{
		std::vector<size_t>{},
		areas_.begin(),
		1
	});

	if (spec_grids_.empty())
		return logic_result::invalid;

	auto flag = false;

	std::vector<uint8_t> atmp(num_areas_, 0x00);
	for (auto &gr : spec_grids_)
	{
		auto it = gr.first.begin();
		auto itt = atmp.begin();
		auto ait = areas_.begin();
		for (; it != gr.first.end(); ++it, ++itt, ++ait)
		{
			if (*it > 0)
				*itt |= 0xf0;
			if (*it < ait->size())
				*itt |= 0x0f;
		}
	}

	{
		auto it = atmp.begin();
		auto ait = areas_.begin();
		for (; it != atmp.end(); ++it, ++ait)
		{
			if (!(*it & 0xf0))
				for (auto b : *ait)
				{
					if (b->is_mine())
						return logic_result::invalid;
					b->set_closed(false);
					flag |= true;
				}

			if (!(*it & 0x0f))
				for (auto b : *ait)
				{
					if (!b->is_mine())
						return logic_result::invalid;
					b->set_closed(false);
					flag |= true;
				}
		}

		return flag ? logic_result::dirty : logic_result::clean;
	}
}

void full_logic::prepare_full_logic()
{
	std::multiset<conn_t, conn_lt> connection;
	for (auto it = grid_.begin(); it != grid_.end(); ++it)
		if (it->is_closed())
			connection.emplace(it);

	areas_.emplace_front();
	auto ait = areas_.begin();
	size_t hash = 0;
	for (auto &conn : connection)
	{
		if (hash != conn.hash)
		{
			if (hash != 0)
			{
				areas_.emplace_back();
				finalize(ait++);
			}
			hash = conn.hash;

			for (const auto b : conn)
			{
				auto nit = neighbors_(b.x(), b.y());
				ait->n_ref.push_back(nit);
				auto &n = *nit;
				if (std::find(n.begin(), n.end(), &*ait) == n.end())
					n.emplace_back(&*ait);
			}
		}
		ait->emplace_back(conn.ref);
	}
	finalize(ait);
}

void full_logic::speculative_fork(fork_directive &&directive)
{
	if (directive.values.size() == num_areas_)
	{
		spec_grids_.emplace_back(std::make_pair(directive.values, enum_stat{ directive.repitition }));
		return;
	}

	const auto ait_sz = directive.ait->size();
	size_t tlb = 0, tub = ait_sz;

	if (config->is_fixed_mines)
	{
		size_t p = 0;
		for (auto &v : directive.values)
			p += v;

		size_t nub = 0;
		auto ait = directive.ait;
		for (++ait; ait != areas_.end(); ++ait)
			nub += ait->size();

		if (grid_st_.rest_mines > p + tub + nub)
			return;
		if (grid_st_.rest_mines < p + tlb + 0)
			return;
		if (grid_st_.rest_mines > p + tlb + nub)
			tlb = grid_st_.rest_mines - p - nub;
		if (grid_st_.rest_mines < p + tub + 0)
			tub = grid_st_.rest_mines - p - 0;
	}

	for (auto &areas : directive.ait->n_ref)
	{
		size_t p = 0;
		size_t nub = 0;
		for (auto ax : *areas)
		{
			if (ax == &*directive.ait)
				continue;

			if (ax->index < directive.ait->index)
				p += directive.values[ax->index];
			else
				nub += directive.ait->size();
		}

		auto b = grid_(areas.x(), areas.y());
		if (b->is_closed())
			throw std::runtime_error("Internal error: should not check closed's neighbor");
		if (b->is_mine())
			throw std::runtime_error("Internal error: should not check mine's neighbor");
		if (b->neighbor() < p || b->neighbor() > p + ait_sz + nub)
			return;

		if (b->neighbor() > p + tub + nub)
			return;
		if (b->neighbor() < p + tlb + 0)
			return;
		if (b->neighbor() > p + tlb + nub)
			tlb = b->neighbor() - p - nub;
		if (b->neighbor() < p + tub + 0)
			tub = b->neighbor() - p - 0;
	}

	auto next_ait = directive.ait;
	++next_ait;

	auto values(directive.values);
	values.push_back(0);
	for (auto i = tlb; i <= tub; i++)
	{
		values.back() = i;
		rep_t lambda = 1;
		if (!config->is_fixed_mines)
		{
			for (size_t j = 1; j <= i; j++)
				lambda *= config->probability;
			for (size_t j = 1; j <= ait_sz - i; j++)
				lambda *= 1 - config->probability;
		}
		speculative_fork(fork_directive{
			values,
			next_ait,
			directive.repitition * binomial(ait_sz, i) * lambda
		});
	}
}

void full_logic::finalize(const area_it &ait)
{
	if (ait->empty())
		return;

	ait->index = num_areas_++;
	for (auto b : *ait)
		*member_(b.x(), b.y()) = &*ait;
}
