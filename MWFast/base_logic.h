#pragma once
#include "common.h"
#include "block.h"
#include "grid.h"
#include "strategy.h"

typedef elem_reference<blk_t> blk_ref;
typedef elem_const_reference<blk_t> blk_const_ref;
typedef std::vector<blk_ref> blk_refs;
typedef std::vector<blk_const_ref> blk_const_refs;

enum class logic_result
{
	clean = 0x00,
	dirty = 0x01,
	invalid = 0x02
};

struct stats
{
	size_t closed;
	size_t rest_mines;
};

struct logic_config
{
	logic_config(bool t, size_t m, double p);
	logic_config(strategy_t &&s, bool t, size_t m, double p);

	strategy_t strategy;
	bool is_fixed_mines;
	size_t total_mines;
	double probability;
};

#define LOGIC(X) do { switch (X) \
{ \
case logic_result::clean: break; \
case logic_result::dirty: flag = true; break; \
case logic_result::invalid: return logic_result::invalid; \
default: throw std::runtime_error("Internal error: unknown logic result"); \
} } while (false)

#define FORCE_LOGIC(X) if ((X) == logic_result::invalid) \
throw std::runtime_error("Internal error: invalid actual grid")
