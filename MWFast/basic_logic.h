#pragma once
#include "common.h"
#include "block.h"
#include "grid.h"
#include "strategy.h"

typedef elem_reference<blk_t> blk_ref;

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
	logic_config(strategy &&s, bool t, size_t m);

	strategy strategy;
	bool is_fixed_mines;
	size_t total_mines;
};

class basic_logic
{
public:
	explicit basic_logic(std::shared_ptr<logic_config> config);

	logic_result try_basic_logic(blk_ref b, bool aggressive);
	logic_result try_basic_logics(grid_t<blk_t> &grid);
	logic_result try_single_logic(blk_ref b, bool aggressive);
	logic_result try_ext_logic(grid_t<blk_t> &grid);

	stats get_stats(const grid_t<blk_t> &grid) const;

	bool is_finished(grid_t<blk_t> &grid) const;

	std::shared_ptr<logic_config> config;
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
