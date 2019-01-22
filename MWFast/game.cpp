#include "game.h"

game::game(const size_t w, const size_t h, const std::string &st)
	: strategy(st), actual_(w, h, blk_t::closed_simple(0)) { }

const grid_t<blk_t> &game::grid() const
{
	return actual_;
}

void game::initialize_mine(blk_ref b)
{
	b->set_mine(true);
	for (auto &n : b.neighbors())
		if (!n->is_mine())
			++*n;
}
