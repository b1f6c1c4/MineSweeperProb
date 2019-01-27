#pragma once
#include "common.h"
#include <vector>

struct strategy_t
{
	enum class logic_method
	{
		zero = 0x00,
		passive = 0x01,
		extended = 0x08,
		single = 0x03,
		dual = 0x07,
		full = 0x1f
	};

	enum class heuristic_method
	{
		min_mine_prob = 0x01,
		max_zero_prob = 0x02,
		max_zeros_prob = 0x03,
		max_zeros_exp = 0x04,
		max_entropy = 0x05,
		min_frontier_dist = 0x06,
		max_upper_bound = 0x07,
		min_mine_prob_est = 0x11,
		max_zero_prob_est = 0x12,
		max_zeros_prob_est = 0x13,
		max_zeros_exp_est = 0x14,
		max_upper_bound_est = 0x17
	};

    logic_method logic;
	size_t initial_x{};
	size_t initial_y{};
    bool heuristic_enabled{};
    std::vector<heuristic_method> decision_tree;
    size_t exhaust_criterion{};

	explicit strategy_t(const std::string &);
};

inline bool operator&(strategy_t::logic_method lhs, strategy_t::logic_method rhs)
{
	// ReSharper disable once CppInconsistentNaming
	using T = std::underlying_type_t<strategy_t::logic_method>;
	return (static_cast<T>(lhs) & static_cast<T>(rhs)) == static_cast<T>(rhs);
}

inline bool operator&(strategy_t::heuristic_method lhs, strategy_t::heuristic_method rhs)
{
	// ReSharper disable once CppInconsistentNaming
	using T = std::underlying_type_t<strategy_t::heuristic_method>;
	return (static_cast<T>(lhs) & static_cast<T>(rhs)) == static_cast<T>(rhs);
}

inline strategy_t::logic_method operator|(strategy_t::logic_method lhs, strategy_t::logic_method rhs)
{
	// ReSharper disable once CppInconsistentNaming
	using T = std::underlying_type_t<strategy_t::logic_method>;
	return static_cast<strategy_t::logic_method>(static_cast<T>(lhs) | static_cast<T>(rhs));
}

inline strategy_t::logic_method &operator|=(strategy_t::logic_method &lhs, strategy_t::logic_method rhs)
{
	// ReSharper disable once CppInconsistentNaming
	using T = std::underlying_type_t<strategy_t::logic_method>;
	lhs = static_cast<strategy_t::logic_method>(static_cast<T>(lhs) | static_cast<T>(rhs));
	return lhs;
}
