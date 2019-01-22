#pragma once
#include "common.h"

class blk_t
{
public:
	blk_t();
	// ReSharper disable once CppNonExplicitConvertingConstructor
	blk_t(uint8_t v);
	uint8_t operator()() const;

	blk_t &operator++();
	blk_t operator++(int);

	static blk_t closed_mine();
	static blk_t closed_simple(uint8_t);

	bool is_front() const;
	blk_t &set_front(bool);

	bool is_closed() const;
	blk_t &set_closed(bool);

	bool is_mine() const;
	blk_t &set_mine(bool);

	uint8_t neighbor() const;
	blk_t &set_neighbor(uint8_t);

	friend std::ostream &operator<<(std::ostream &os, const blk_t &b);
private:
	uint8_t value_;
};
