#include "strategy.h"
#include <sstream>

bool read_strategy(const std::string &hsh, strategy_t &st)
{
	std::stringstream ss(hsh);

#define CHK(s) if (ch != s) return false
#define NXT(s) do { if (!(ss >> ch)) return false; CHK(s); } while (false)
#define GET(ch) if (!(ss >> ch)) ch = '\0'

	char ch;
	GET(ch);
	switch (ch)
	{
	case 'Z':
		st.logic = strategy_t::logic_method::zero;
		break;
	case 'P':
		st.logic = strategy_t::logic_method::passive;
		break;
	case 'S':
		st.logic = strategy_t::logic_method::single;
		break;
	case 'D':
		st.logic = strategy_t::logic_method::dual;
		break;
	case 'F':
		st.logic = strategy_t::logic_method::full;
		break;
	default:
		return false;
	}
	NXT('L');
	GET(ch);
	if (ch == 'E')
	{
		st.logic |= strategy_t::logic_method::extended;
		GET(ch);
	}

	CHK('@');
	NXT('[');
	if (!(ss >> st.initial_x))
		return false;
	NXT(',');
	if (!(ss >> st.initial_y))
		return false;
	NXT(']');
	st.initial_x--;
	st.initial_y--;
	NXT('-');

	st.heuristic_enabled = true;
	GET(ch);
	if (ch == 'N')
		NXT('H');
	else if (ch == 'P')
	{
		GET(ch);
		if (ch == 'u')
		{
			NXT('r');
			NXT('e');
			st.heuristic_enabled = false;
		}
		if (st.heuristic_enabled)
			st.decision_tree.push_back(strategy_t::heuristic_method::min_mine_prob);
	}
	while (ch != '-' && ch != '\0')
	{
		switch (ch)
		{
		case 'p':
			st.decision_tree.push_back(strategy_t::heuristic_method::min_area_max_prob);
			break;
		case 'P':
			st.decision_tree.push_back(strategy_t::heuristic_method::min_mine_prob);
			break;
		case 'Z':
			st.decision_tree.push_back(strategy_t::heuristic_method::max_zero_prob);
			break;
		case 'S':
			st.decision_tree.push_back(strategy_t::heuristic_method::max_zeros_prob);
			break;
		case 'E':
			st.decision_tree.push_back(strategy_t::heuristic_method::max_zeros_exp);
			break;
		case 'Q':
			st.decision_tree.push_back(strategy_t::heuristic_method::max_entropy);
			break;
		case 'F':
			st.decision_tree.push_back(strategy_t::heuristic_method::min_frontier_dist);
			break;
		case 'U':
			st.decision_tree.push_back(strategy_t::heuristic_method::max_upper_bound);
			break;
		default:
			break;
		}
		GET(ch);
	}

	if (ch == '\0')
	{
		st.exhaust_criterion = 0;
		return true;
	}

	GET(ch);
	if (ch != 'D')
		return false;
	if (!(ss >> st.exhaust_criterion))
		return false;

	return true;
}

strategy_t::strategy_t(const std::string &hsh)
{
	if (!read_strategy(hsh, *this))
		throw std::runtime_error("Invalid strategy");
}
