#pragma once
#include "common.h"
#include "basic_logic.h"
#include <list>
#include <set>

struct enum_stat
{
	rep_t repitition;
};

typedef std::pair<std::vector<size_t>, enum_stat> spec_t;

struct area;

typedef std::vector<area*> area_refs;

struct area : std::vector<blk_ref>
{
	size_t index = 0;
	std::vector<elem_reference<area_refs>> n_ref;
};

typedef std::list<area>::iterator area_it;

struct blk_ref_lt
{
	bool operator()(const blk_ref &lhs, const blk_ref &rhs) const;
};

struct conn_t : std::set<blk_ref, blk_ref_lt>
{
	size_t hash;
	blk_ref ref;

	explicit conn_t(blk_ref b);
};
struct conn_lt
{
	bool operator()(const conn_t &lhs, const conn_t &rhs) const;
};

struct topo_t
{
	area *my_area;
	area_refs neighbors_areas;
};

class full_logic : protected basic_logic
{
public:
	full_logic(std::shared_ptr<grid_t<blk_t>> grid, std::shared_ptr<logic_config> config);

	// topo_t get_topology(blk_const_ref b) const;
	full_logic logic_fork_spec(blk_const_ref b, uint8_t n) const;
	void modify_spec(blk_const_ref b, uint8_t n);

	logic_result try_full_logics(blk_ref pivot, bool spec = false);
	logic_result try_full_logics(bool spec = false);

	const grid_t<blk_t> &actual() const;
	const std::list<area> &areas() const;
	const std::vector<spec_t> &specs() const;
	std::shared_ptr<logic_config> get_config() const;
	size_t safe_count() const;
	rep_t rep_count() const;

private:
	struct fork_directive
	{
		std::vector<size_t> values;
		area_it ait;
		rep_t repitition;
	};

	logic_result try_full_logic();
	void prepare_full_logic();
	void speculative_fork(fork_directive &&directive);

	std::shared_ptr<grid_t<blk_t>> grid_;
	grid_t<blk_t> simp_grid_;
	stats grid_st_;
	grid_t<area*> member_;
	grid_t<area_refs> neighbors_;
	std::list<area> areas_;
	size_t num_areas_;
	std::vector<spec_t> spec_grids_;
	bool is_speculative_;
	size_t safe_count_;
	rep_t rep_count_;

	void finalize(const area_it &ait);
	static area *emplace_fork(full_logic &logic, const area &a, const blk_const_refs &bs);
};
