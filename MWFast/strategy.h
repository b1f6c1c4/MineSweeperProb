#pragma once
#include "common.h"
#include <vector>

struct strategy
{
	enum class logic_method
	{
		zero = 0x00,
		passive = 0x01,
		single = 0x03,
		full = 0x0b
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

inline bool operator&(strategy::logic_method lhs, strategy::logic_method rhs)
{
	// ReSharper disable once CppInconsistentNaming
	using T = std::underlying_type_t<strategy::logic_method>;
	return (static_cast<T>(lhs) & static_cast<T>(rhs)) == static_cast<T>(rhs);
}

inline bool operator&(strategy::heuristic_method lhs, strategy::heuristic_method rhs)
{
	// ReSharper disable once CppInconsistentNaming
	using T = std::underlying_type_t<strategy::heuristic_method>;
	return (static_cast<T>(lhs) & static_cast<T>(rhs)) == static_cast<T>(rhs);
}
