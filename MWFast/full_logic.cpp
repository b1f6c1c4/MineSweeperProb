#include "full_logic.h"
#include "binomial.h"
#include "lp.h"

bool blk_ref_lt::operator()(const blk_ref &lhs, const blk_ref &rhs) const
{
	return &*lhs < &*rhs;
}

conn_t::conn_t(blk_ref b): hash(0), ref(b)
{
	for (auto bb : b.neighbors())
		if (!bb->is_closed() && !bb->is_spec() && !bb->is_mine())
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

full_logic::full_logic(std::shared_ptr<grid_t<blk_t>> grid, std::shared_ptr<logic_config> config)
	: basic_logic(config), grid_(grid),
	  simp_grid_(grid->width(), grid->height(), 0),
	  grid_st_{}, member_(grid->width(), grid->height(), nullptr),
	  neighbors_(grid->width(), grid->height(), std::vector<area*>{}), num_areas_(0),
	  is_speculative_(false), safe_count_(0), rep_count_(0), lp_(nullptr) { }

area *full_logic::emplace_fork(full_logic &logic, const area &a, const blk_const_refs &bs)
{
	logic.areas_.emplace_back();
	auto &new_area = logic.areas_.back();
	new_area.index = logic.num_areas_++;
	for (auto bb : bs)
	{
		new_area.push_back((*logic.grid_)(bb.x(), bb.y()));
		*logic.member_(bb.x(), bb.y()) = &new_area;
	}
	for (const auto nr : a.n_ref)
	{
		auto new_nr = logic.neighbors_(nr.x(), nr.y());
		new_area.n_ref.push_back(new_nr);
		new_nr->push_back(&new_area);
	}
	return &new_area;
}

full_logic full_logic::logic_fork_spec(blk_const_ref b, const uint8_t n) const
{
	const auto grid = std::make_shared<grid_t<blk_t>>(b.grid());
	(*grid)(b.x(), b.y())->set_mine(false).set_spec(false).set_closed(false).set_neighbor(n);

	full_logic logic(grid, config);
	logic.is_speculative_ = true;
	logic.simp_grid_ = simp_grid_;
	auto nx = n;
	for (const auto bn : b.neighbors())
		if (!bn->is_closed() && bn->is_mine())
			nx--;
	logic.simp_grid_(b.x(), b.y())->set_mine(false).set_spec(false).set_closed(false).set_neighbor(nx);
	logic.grid_st_ = get_stats(*grid);
	logic.num_areas_ = 0;

	for (const auto &a : areas_)
	{
		blk_const_refs bns, bfs;
		for (const blk_const_ref bb : a)
		{
			if (bb == b)
				continue;

			auto flag = false;
			for (const auto bn : b.neighbors()) // TODO
				if (bb == bn)
				{
					bns.push_back(bb);
					flag = true;
					break;
				}

			if (!flag)
				bfs.push_back(bb);
		}

		if (!bns.empty())
		{
			auto res = emplace_fork(logic, a, bns);
			auto new_nr = logic.neighbors_(b.x(), b.y());
			res->n_ref.push_back(new_nr);
			new_nr->push_back(res);
		}

		if (!bfs.empty())
			// ReSharper disable once CppExpressionWithoutSideEffects
			emplace_fork(logic, a, bfs);
	}
	return logic;
}

void full_logic::modify_spec(blk_const_ref b, const uint8_t n)
{
	auto bg = (*grid_)(b.x(), b.y());
	auto bx = simp_grid_(b.x(), b.y());
	bx->set_neighbor(n + bx->neighbor() - bg->neighbor());
	bg->set_neighbor(n);
}

logic_result full_logic::try_full_logics(const blk_ref pivot, const bool spec)
{
	auto flag = false;
	LOGIC(try_basic_logic(pivot, true));
	LOGIC(try_full_logics(spec));
	return flag ? logic_result::dirty : logic_result::clean;
}

logic_result full_logic::try_full_logics(const bool spec)
{
	if (spec)
	{
		// *this should be prepared by full_logic::fork. Don't prepare again.
		if (try_full_logic() == logic_result::invalid)
			return logic_result::invalid;
		return logic_result::clean;
	}

	bool flag;
	do
	{
		flag = false;
		LOGIC(try_basic_logics(*grid_));

		if (!(config->strategy.logic & strategy_t::logic_method::full))
			return logic_result::clean;

		prepare_full_logic();
		if (num_areas_)
			LOGIC(try_full_logic());
	}
	while (flag);
	return logic_result::clean;
}

const grid_t<blk_t> &full_logic::actual() const
{
	return *grid_;
}

const std::list<area> &full_logic::areas() const
{
	return areas_;
}

const std::vector<spec_t> &full_logic::specs() const
{
	return spec_grids_;
}

std::shared_ptr<logic_config> full_logic::get_config() const
{
	return config;
}

size_t full_logic::safe_count() const
{
	return safe_count_;
}

rep_t full_logic::rep_count() const
{
	return rep_count_;
}

const lp<area, list_simple> & full_logic::lp_solver() const
{
	return *lp_;
}

const grid_t<area *> &full_logic::member() const
{
	return member_;
}

logic_result full_logic::try_full_logic()
{
	spec_grids_.clear();
	safe_count_ = 0;
	rep_count_ = 0;

#ifndef NDEBUG
	if (num_areas_ > 30)
	{
		std::cerr << "DEPTH = " << num_areas_ << std::endl;
		std::cerr << *grid_;
	}
#endif

	size_t cons = 0;
	if (config->is_fixed_mines)
		cons++;
	{
		auto it = simp_grid_.begin();
		auto itn = neighbors_.begin();
		for (; it != simp_grid_.end(); ++it, ++itn)
			if (!it->is_closed() && !it->is_mine() && !itn->empty())
				++cons;
	}

	if (lp_ == nullptr)
		lp_ = make_lp(areas_, num_areas_, cons);
	else
		lp_->reset(num_areas_, cons);

	if (config->is_fixed_mines)
		lp_->constraint(grid_st_.rest_mines);
	{
		auto it = simp_grid_.begin();
		auto itn = neighbors_.begin();
		for (; it != simp_grid_.end(); ++it, ++itn)
			if (!it->is_closed() && !it->is_mine())
				if (!itn->empty())
					lp_->constraint(*itn, it->neighbor());
	}

	const auto result = lp_->solve();
	if (result == logic_result::invalid)
		return logic_result::invalid;

	if (result == logic_result::clean)
		return logic_result::clean;

	{
		auto flag = false;
		auto it = lp_->get_result().begin();
		auto ait = areas_.begin();
		for (; ait != areas_.end(); ++it, ++ait)
		{
			if (it->second == 0)
				for (auto b : *ait)
				{
					safe_count_++;
					if (is_speculative_)
						continue;

					if (b->is_mine())
						return logic_result::invalid;
					b->set_closed(false);
					b->set_mine(false);
					flag |= true;
				}

			if (it->first == ait->size())
				for (auto b : *ait)
				{
					if (is_speculative_)
						continue;
					if (!b->is_mine())
						return logic_result::invalid;
					b->set_closed(false);
					b->set_mine(true);
					flag |= true;
				}
		}

		return flag ? logic_result::dirty : logic_result::clean;
	}
}

void full_logic::prepare_full_logic()
{
	simp_grid_ = *grid_;
	for (auto it = simp_grid_.begin(); it != simp_grid_.end(); ++it)
		if (it->is_closed())
			it->set_spec(true).set_mine(false).set_neighbor(0);
		else if (!it->is_mine())
			for (auto b : it.neighbors())
				if (!b->is_closed() && b->is_mine())
				{
					if (it->neighbor() == 0)
						throw std::runtime_error("Internal error: no enough neighbor");
					it->set_neighbor(it->neighbor() - 1);
				}

	grid_st_ = get_stats(*grid_);
	num_areas_ = 0;
	areas_.clear();
	for (auto &n : neighbors_)
		n.clear();

	std::multiset<conn_t, conn_lt> connection;
	for (auto it = grid_->begin(); it != grid_->end(); ++it)
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
		spec_grids_.emplace_back(std::make_pair(directive.values, enum_stat{directive.repitition}));
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
				nub += ax->size();
		}

		auto b = simp_grid_(areas.x(), areas.y());
		if (b->is_closed())
			throw std::runtime_error("Internal error: should not check closed's neighbor");
		if (b->is_mine())
			throw std::runtime_error("Internal error: should not check mine's neighbor");
		if (b->is_spec())
			continue;

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
