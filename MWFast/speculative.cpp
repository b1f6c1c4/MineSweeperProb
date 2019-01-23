#include "speculative.h"

spec_grid_t::spec_grid_t(const grid_t<blk_t> &o, const rep_t r) : grid_t<blk_t>(o), repitition(r) { }
