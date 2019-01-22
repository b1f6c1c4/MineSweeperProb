#include "game.h"

game::game(const size_t w, const size_t h, const std::string &st)
	: strategy(st), actual_(w, h, blk_t::closed_simple(0)) { }

const grid_t<blk_t> &game::grid() const
{
	return actual_;
}

void game::fill_fixed(size_t total_mines, const bool buff)
{
	const std::uniform_int_distribution<size_t> dx(0, actual_.width() - 1);
	const std::uniform_int_distribution<size_t> dy(0, actual_.height() - 1);

	while (total_mines)
	{
		const auto x = dx(device);
		const auto y = dy(device);

		if (strategy.initial_x == x && strategy.initial_y == y)
			continue;

		if (buff)
		{
			if (strategy.initial_x - x <= 1ull ||
				x - strategy.initial_x <= 1ull)
				if (strategy.initial_y - y <= 1ull ||
					y - strategy.initial_y <= 1ull)
					continue;
		}

		auto b = actual_(x, y);
		if (b->is_mine())
			continue;
		initialize_mine(b);
		--total_mines;
	}
}

void game::fill_prob(const double probability, const bool buff)
{
	const std::bernoulli_distribution d(probability);

	for (auto it = actual_.begin(); it != actual_.end(); ++it)
	{
		const auto x = it.x(), y = it.y();

		if (strategy.initial_x == x && strategy.initial_y == y)
			continue;

		if (buff)
		{
			if (strategy.initial_x - x <= 1ull ||
				x - strategy.initial_x <= 1ull)
				if (strategy.initial_y - y <= 1ull ||
					y - strategy.initial_y <= 1ull)
					continue;
		}

		const auto result = d(device);
		if (result)
			initialize_mine(it);
	}
}

bool game::run()
{
	auto init = actual_(strategy.initial_x, strategy.initial_y);
	if (init->is_closed())
	{
		actual_open(init);
		try_basic_logic(init, true);
		try_basic_logics();
		try_full_logic();
	}

	return is_finished();
}

bool game::is_finished() const
{
	for (const auto &b : actual_)
		if (b.is_closed())
			return false;

	return true;
}

void game::actual_open(blk_ref b)
{
	if (!b->is_closed())
		throw std::runtime_error("Internal error: try to open a opened block");

	if (b->is_mine())
		throw std::runtime_error("Internal error: try to logically open a mine");

	b->set_closed(false);
	if (b->is_front())
		b->set_front(false), front_size_--;
}

bool game::try_basic_logic(blk_ref b, const bool aggressive)
{
	if (b->is_closed())
		throw std::runtime_error("Internal error: try to logic a closed block");

	auto flag = false;

	if (strategy.logic & strategy::logic_method::passive)
	{
		if (!b->is_mine() && b->neighbor() == 0)
			for (const auto bb : b.neighbors())
				if (bb->is_closed())
				{
					actual_open(bb);
					flag = true;
					try_basic_logic(bb, aggressive);
				}
	}

	if (strategy.logic & strategy::logic_method::single)
	{
		if (!b->is_mine())
			flag |= try_single_logic(b, aggressive);
		else if (aggressive)
			for (const auto bb : b.neighbors())
				if (!bb->is_mine())
					flag |= try_single_logic(bb, false);
	}

	return flag;
}

void game::try_basic_logics()
{
	bool flag;
	do
	{
		flag = false;
		for (auto it = actual_.begin(); it != actual_.end(); ++it)
			if (!it->is_closed())
				flag |= try_basic_logic(it, false);
	} while (flag);
}

bool game::try_single_logic(blk_ref b, const bool aggressive)
{
	if (b->is_mine())
		throw std::runtime_error("Internal error: try single logic on a mine");

	auto flag = false;

	size_t cnt = 0, cntm = 0;
	for (const auto bb : b.neighbors())
	{
		if (bb->is_closed())
			cnt++;
		else if (bb->is_mine())
			cntm++;
	}
	if (b->neighbor() < cntm)
		throw std::runtime_error("Internal error: too few mines");

	if (b->neighbor() == cntm)
	{
		for (const auto bb : b.neighbors())
			if (bb->is_closed())
			{
				actual_open(bb);
				flag = true;
				try_basic_logic(bb, aggressive);
			}
	}
	else if (b->neighbor() == cntm + cnt)
	{
		for (auto bb : b.neighbors())
			if (bb->is_closed())
			{
				bb->set_closed(false);
				bb->set_mine(true);
				flag = true;
				try_basic_logic(bb, aggressive);
			}
	}

	return flag;
}

void game::try_full_logic()
{
	if (!(strategy.logic & strategy::logic_method::full))
		return;

	prepare_full_logic();
	// TODO
}

void game::prepare_full_logic()
{
	for (auto it = actual_.begin(); it != actual_.end(); ++it)
		if (!it->is_closed())
			for (auto b : it.neighbors())
				if (b->is_closed() && !b->is_front())
					b->set_front(true), front_size_++;
}

void game::initialize_mine(blk_ref b)
{
	b->set_mine(true);
	for (auto &n : b.neighbors())
		if (!n->is_mine())
			++*n;
}
