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

class enumerator_logic : protected basic_logic
{
public:
	enumerator_logic(grid_t<blk_t> &grid, std::shared_ptr<logic_config> config);

private:
	struct fork_directive
	{
		std::vector<size_t> values;
		area_it ait;
		rep_t repitition;
	};

	logic_result try_full_logic(bool force = false);
	void speculative_fork(fork_directive &&directive);

	grid_t<blk_t> &grid_;
	stats grid_st_;
	grid_t<area*> member_;
	grid_t<area_refs> neighbors_;
	std::list<area> areas_;
	size_t num_areas_;
	std::vector<spec_t> spec_grids_;

	void finalize(const area_it &ait);
};
