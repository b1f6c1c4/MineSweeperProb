#pragma once
#include "common.h"
#include <vector>

struct strategy
{
	enum class logic_method
	{
		passive = 0x00,
		single = 0x01,
		single_extended = 0x02,
		full = 0x05
	};

	enum class heuristic_method
	{
		min_mine_prob = 0x01,
		max_zero_prob = 0x02,
		max_zeros_prob = 0x03,
		max_zeros_exp = 0x04,
		max_quantity_exp = 0x05,
		min_frontier_dist = 0x06,
		max_upper_bound = 0x07
	};

    logic_method logic;
	size_t initial_x;
	size_t initial_y;
    bool heuristic_enabled;
    std::vector<heuristic_method> decision_tree;
    size_t exhaust_criterion;

	explicit strategy(const std::string &);
};
