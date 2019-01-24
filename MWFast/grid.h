#pragma once
#include "common.h"
#include <algorithm>
#include <iostream>

template <typename T>
class grid_t;

template <typename T>
class neighbors_t;

template <typename T>
class const_neighbors_t;

template <typename T>
class elem_const_reference;

template <typename T>
class elem_reference
{
public:
	elem_reference() : x_(0), y_(0), value_(nullptr), grid_(nullptr) { }

	elem_reference(grid_t<T> *g, T *v, const size_t x, const size_t y)
		: x_(x), y_(y), value_(v), grid_(g) { }

	T &operator*() { return *value_; }
	const T &operator*() const { return *value_; }
	T *operator->() { return value_; }
	const T *operator->() const { return value_; }

	neighbors_t<T> neighbors() { return neighbors_t<T>(*grid_, x_, y_); }
	const_neighbors_t<T> neighbors() const { return const_neighbors_t<T>(*grid_, x_, y_); }

	operator elem_const_reference<T>() const { return elem_const_reference<T>(grid_, value_, x_, y_); }

	grid_t<T> &grid() { return *grid_; }
	size_t x() const { return x_; }
	size_t y() const { return y_; }

	template <typename U>
	friend bool operator==(const elem_reference<U> &lhs, const elem_reference<U> &rhs);
	template <typename U>
	friend bool operator!=(const elem_reference<U> &lhs, const elem_reference<U> &rhs);
protected:
	size_t x_, y_;
	T *value_;
	grid_t<T> *grid_;
};

template <typename T>
class elem_const_reference
{
public:
	elem_const_reference() : x_(0), y_(0), value_(nullptr), grid_(nullptr) { }

	elem_const_reference(const grid_t<T> *g, const T *v, const size_t x, const size_t y)
		: x_(x), y_(y), value_(v), grid_(g) { }

	const T &operator*() const { return *value_; }
	const T *operator->() const { return value_; }

	const_neighbors_t<T> neighbors() const { return const_neighbors_t<T>(*grid_, x_, y_); }

	const grid_t<T> &grid() const { return *grid_; }
	size_t x() const { return x_; }
	size_t y() const { return y_; }

	template <typename U>
	friend bool operator==(const elem_const_reference<U> &lhs, const elem_const_reference<U> &rhs);
	template <typename U>
	friend bool operator!=(const elem_const_reference<U> &lhs, const elem_const_reference<U> &rhs);
protected:
	size_t x_, y_;
	const T *value_;
	const grid_t<T> *grid_;
};

template <typename T>
bool operator==(const elem_reference<T> &lhs, const elem_reference<T> &rhs)
{
	return lhs.value_ == rhs.value_;
}

template <typename T>
bool operator!=(const elem_reference<T> &lhs, const elem_reference<T> &rhs)
{
	return lhs.value_ != rhs.value_;
}

template <typename T>
bool operator==(const elem_const_reference<T> &lhs, const elem_const_reference<T> &rhs)
{
	return lhs.value_ == rhs.value_;
}

template <typename T>
bool operator!=(const elem_const_reference<T> &lhs, const elem_const_reference<T> &rhs)
{
	return lhs.value_ != rhs.value_;
}

template <typename TGrid, typename TTarget>
size_t construct_neighbor(TGrid &grid, size_t x, size_t y, TTarget &neighbors)
{
	size_t size = 0;
	if (x > 0)
	{
		if (y > 0)
			neighbors[size++] = grid(x - 1, y - 1);
		neighbors[size++] = grid(x - 1, y);
		if (y < grid.height() - 1)
			neighbors[size++] = grid(x - 1, y + 1);
	}
	if (y > 0)
		neighbors[size++] = grid(x, y - 1);
	if (y < grid.height() - 1)
		neighbors[size++] = grid(x, y + 1);
	if (x < grid.width() - 1)
	{
		if (y > 0)
			neighbors[size++] = grid(x + 1, y - 1);
		neighbors[size++] = grid(x + 1, y);
		if (y < grid.height() - 1)
			neighbors[size++] = grid(x + 1, y + 1);
	}
	return size;
}

template <typename T>
class neighbors_t
{
	typedef std::array<elem_reference<T>, 8> storage;
public:
	neighbors_t(grid_t<T> &grid, size_t x, size_t y) : size_(construct_neighbor(grid, x, y, neighbors_)) { }

	typedef typename storage::iterator iterator;
	typedef typename storage::const_iterator const_iterator;

	iterator begin() { return neighbors_.begin(); }
	iterator end() { return neighbors_.begin() + size_; }
	const_iterator begin() const { return neighbors_.begin(); }
	const_iterator end() const { return neighbors_.begin() + size_; }
private:
	storage neighbors_;
	size_t size_;
};

template <typename T>
class const_neighbors_t
{
	typedef std::array<elem_const_reference<T>, 8> storage;
public:
	const_neighbors_t(const grid_t<T> &grid, size_t x, size_t y) : size_(construct_neighbor(grid, x, y, neighbors_)) { }

	typedef typename storage::const_iterator const_iterator;

	const_iterator begin() const { return neighbors_.begin(); }
	const_iterator end() const { return neighbors_.begin() + size_; }
private:
	storage neighbors_;
	size_t size_;
};

template <typename T>
class grid_iterator : public elem_reference<T>
{
public:
	grid_iterator(grid_t<T> *g, T *v, const size_t x, const size_t y)
		: elem_reference<T>(g, v, x, y) { }

	grid_iterator &operator++()
	{
		++elem_reference<T>::value_;
		if (++elem_reference<T>::x_ >= elem_reference<T>::grid_->width())
		{
			elem_reference<T>::x_ -= elem_reference<T>::grid_->width();
			++elem_reference<T>::y_;
		}
		return *this;
	}

	grid_iterator operator++(int)
	{
		auto &&temp = grid_iterator(elem_reference<T>::grid_, elem_reference<T>::value_,
		                            elem_reference<T>::x_, elem_reference<T>::y_);
		++*this;
		return temp;
	}

	bool operator==(const grid_iterator &rhs) { return elem_reference<T>::value_ == rhs.value_; }
	bool operator!=(const grid_iterator &rhs) { return elem_reference<T>::value_ != rhs.value_; }
};

template <typename T>
class grid_const_iterator : public elem_const_reference<T>
{
public:
	grid_const_iterator(const grid_t<T> *g, const T *v, const size_t x, const size_t y)
		: elem_const_reference<T>(g, v, x, y) { }

	grid_const_iterator &operator++()
	{
		++elem_const_reference<T>::value_;
		if (++elem_const_reference<T>::x_ >= elem_const_reference<T>::grid_->width())
		{
			elem_const_reference<T>::x_ -= elem_const_reference<T>::grid_->width();
			++elem_const_reference<T>::y_;
		}
		return *this;
	}

	grid_const_iterator operator++(int)
	{
		auto &&temp = grid_const_iterator(elem_const_reference<T>::grid_, elem_const_reference<T>::value_,
		                                  elem_const_reference<T>::x_, elem_const_reference<T>::y_);
		++*this;
		return temp;
	}

	bool operator==(const grid_const_iterator &rhs) { return elem_const_reference<T>::value_ == rhs.value_; }
	bool operator!=(const grid_const_iterator &rhs) { return elem_const_reference<T>::value_ != rhs.value_; }
};

#ifdef FIXED_GRID_SIZE

template <typename T>
class grid_t
{
public:
	grid_t(const size_t w, const size_t h, const T &value)
	{
		if (w != WIDTH || h != HEIGHT)
			throw std::runtime_error("Grid size incorrect");

		std::fill(grid_.begin(), grid_.end(), value);
	}

	elem_reference<T> operator()(const size_t x, const size_t y)
	{
		return elem_reference<T>(this, &grid_[y * WIDTH + x], x, y);
	}

	elem_const_reference<T> operator()(const size_t x, const size_t y) const
	{
		return elem_const_reference<T>(this, &grid_[y * WIDTH + x], x, y);
	}

	// ReSharper disable CppMemberFunctionMayBeStatic
	size_t width() const { return WIDTH; }
	size_t height() const { return HEIGHT; }
	// ReSharper restore CppMemberFunctionMayBeStatic

	typedef grid_iterator<T> iterator;
	typedef grid_const_iterator<T> const_iterator;

	iterator begin() { return iterator(this, &*grid_.begin(), 0, 0); }
	iterator end() { return iterator(this, &*grid_.begin() + WIDTH * HEIGHT, 0, HEIGHT); }
	const_iterator begin() const { return const_iterator(this, &*grid_.begin(), 0, 0); }
	const_iterator end() const { return const_iterator(this, &*grid_.begin() + WIDTH * HEIGHT, 0, HEIGHT); }

	template <typename U>
	friend std::ostream &operator<<(std::ostream &os, const grid_t<U> &grid);
private:
	std::array<T, WIDTH * HEIGHT> grid_;
};

#else

template <typename T>
class grid_t
{
public:
	grid_t(const size_t w, const size_t h, const T &value) : grid_(0), width_(w), height_(h)
	{
		grid_.resize(w * h, value);
	}

	elem_reference<T> operator()(const size_t x, const size_t y)
	{
		return elem_reference<T>(this, &grid_[y * width_ + x], x, y);
	}

	elem_const_reference<T> operator()(const size_t x, const size_t y) const
	{
		return elem_const_reference<T>(this, &grid_[y * width_ + x], x, y);
	}

	size_t width() const { return width_; }
	size_t height() const { return height_; }

	typedef grid_iterator<T> iterator;
	typedef grid_const_iterator<T> const_iterator;

	iterator begin() { return iterator(this, &*grid_.begin(), 0, 0); }
	iterator end() { return iterator(this, &*grid_.begin() + width_ * height_, 0, height_); }
	const_iterator begin() const { return const_iterator(this, &*grid_.begin(), 0, 0); }
	const_iterator end() const { return const_iterator(this, &*grid_.begin() + width_ * height_, 0, height_); }

	template <typename U>
	friend std::ostream &operator<<(std::ostream &os, const grid_t<U> &grid);
private:
	std::vector<T> grid_;
	size_t width_, height_;
};

#endif

template <typename U>
std::ostream &operator<<(std::ostream &os, const grid_t<U> &grid)
{
	for (auto it = grid.begin(); it != grid.end(); ++it)
	{
		if (it.x() != 0)
			os << " ";
		os << *it;
		if (it.x() == grid.width() - 1)
			os << std::endl;
	}
	return os;
}
