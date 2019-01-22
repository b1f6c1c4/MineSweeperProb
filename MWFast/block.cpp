#include "block.h"

blk_t::blk_t() : value_(0) { }

blk_t::blk_t(const uint8_t v) : value_(v) { }

uint8_t blk_t::operator()() const
{
	return value_;
}

blk_t &blk_t::operator++()
{
	++value_;
	return *this;
}

blk_t blk_t::operator++(int)
{
	return value_++;
}

bool blk_t::is_front() const
{
	return value_ & 0x40;
}

blk_t &blk_t::set_front(const bool v)
{
	value_ &= ~0x40;
	value_ |= v ? 0x40 : 0x00;
	return *this;
}

bool blk_t::is_closed() const
{
	return value_ & 0x20;
}

blk_t &blk_t::set_closed(const bool v)
{
	value_ &= ~0x20;
	value_ |= v ? 0x20 : 0x00;
	return *this;
}

bool blk_t::is_mine() const
{
	return value_ & 0x10;
}

blk_t &blk_t::set_mine(const bool v)
{
	if (v)
	{
		value_ &= ~0x0f;
		value_ |= 0x10;
	}
	else
		value_ &= ~0x10;
	return *this;
}

uint8_t blk_t::neighbor() const
{
	return value_ & 0x0f;
}

blk_t &blk_t::set_neighbor(const uint8_t v)
{
	value_ &= ~0x40;
	value_ |= v;
	return *this;
}

blk_t blk_t::closed_mine()
{
	return 0x30;
}

blk_t blk_t::closed_simple(const uint8_t v)
{
	return 0x20 | v;
}

std::ostream &operator<<(std::ostream &os, const blk_t &b)
{
	if (!b.is_closed())
	{
		if (b.is_mine())
			os << 'M';
		else
			os << static_cast<size_t>(b.neighbor());
	}
	else if (b.is_front())
	{
		if (b.is_mine())
			os << '+';
		else
			os << '-';
	}
	else
	{
		if (b.is_mine())
			os << '.';
		else
			os << ' ';
	}
	return os;
}
