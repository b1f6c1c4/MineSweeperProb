#pragma once
#include "common.h"
#include <algorithm>
#include <iostream>

template <typename T>
class grid;

template <typename TGrid, typename TTarget>
size_t construct_neighbor(TGrid &grid, size_t x, size_t y, TTarget &neighbors)
{
	size_t size = 0;
	if (x > 0)
	{
		if (y > 0)
			neighbors[size++] = &grid(x - 1, y - 1);
		neighbors[size++] = &grid(x - 1, y);
		if (y < grid.width() - 1)
			neighbors[size++] = &grid(x - 1, y + 1);
	}
	if (y > 0)
		neighbors[size++] = &grid(x, y - 1);
	if (y < grid.width() - 1)
		neighbors[size++] = &grid(x, y + 1);
	if (x < grid.height() - 1)
	{
		if (y > 0)
			neighbors[size++] = &grid(x + 1, y - 1);
		neighbors[size++] = &grid(x + 1, y);
		if (y < grid.width() - 1)
			neighbors[size++] = &grid(x + 1, y + 1);
	}
	return size;
}

template <typename T>
class neighbors_iterator
{
public:
	typedef neighbors_iterator self_type;
	typedef T value_type;
	typedef std::forward_iterator_tag iterator_category;
	// ReSharper disable once CppNonExplicitConvertingConstructor
	neighbors_iterator(T **ptr) : ptr_(ptr) { }
	self_type operator++() { ++ptr_; return *this; }
	self_type operator++(int) { self_type i = *this; ++ptr_; return i; }
	T &operator*() { return **ptr_; }
	T *operator->() { return *ptr_; }
	bool operator==(const self_type &rhs) { return ptr_ == rhs.ptr_; }
	bool operator!=(const self_type &rhs) { return ptr_ != rhs.ptr_; }
private:
	T **ptr_;
};

template <typename T>
class const_neighbors_iterator
{
public:
	typedef const_neighbors_iterator self_type;
	typedef T value_type;
	typedef std::forward_iterator_tag iterator_category;
	// ReSharper disable once CppNonExplicitConvertingConstructor
	const_neighbors_iterator(const T * const *ptr) : ptr_(ptr) { }
	self_type operator++() { ++ptr_; return *this; }
	self_type operator++(int) { self_type i = *this; ++ptr_; return i; }
	const T &operator*() { return **ptr_; }
	const T *operator->() { return *ptr_; }
	bool operator==(const self_type &rhs) { return ptr_ == rhs.ptr_; }
	bool operator!=(const self_type &rhs) { return ptr_ != rhs.ptr_; }
private:
	const T * const *ptr_;
};

template <typename T>
class neighbors_t
{
public:
	neighbors_t(grid<T> &grid, size_t x, size_t y) : size_(construct_neighbor(grid, x, y, neighbors_)) { }

	typedef neighbors_iterator<T> iterator;
	typedef const_neighbors_iterator<T> const_iterator;

	iterator begin() { return neighbors_.front(); }
	iterator end() { return neighbors_.front() + size_; }
	const_iterator begin() const { return neighbors_.front(); }
	const_iterator end() const { return neighbors_.front() + size_; }
private:
	std::array<T*, 8> neighbors_;
	size_t size_;
};

template <typename T>
class const_neighbors_t
{
public:
	const_neighbors_t(grid<T> &grid, size_t x, size_t y) : size_(construct_neighbor(grid, x, y, neighbors_)) { }

	typedef const_neighbors_iterator<T> const_iterator;

	const_iterator begin() const { return neighbors_.front(); }
	const_iterator end() const { return neighbors_.front() + size_; }
private:
	std::array<const T*, 8> neighbors_;
	size_t size_;
};

#ifdef FIXED_GRID_SIZE

template <typename T>
class grid
{
public:
	grid(const size_t w, const size_t h, const T &value)
	{
		if (w != WIDTH || h != HEIGHT)
			throw std::runtime_error("Grid size incorrect");

		std::fill(grid_.begin(), grid_.end(), value);
	}

	T &operator()(const size_t x, const size_t y) { return grid_[y * WIDTH + x]; }
	const T &operator()(const size_t x, const size_t y) const { return grid_[y * WIDTH + x]; }

	neighbors_t<T> neighbors(const size_t x, const size_t y) { return neighbors_t<T>(*this, x, y); }
	const_neighbors_t<T> neighbors(const size_t x, const size_t y) const { return const_neighbors_t<T>(*this, x, y); }

	// ReSharper disable CppMemberFunctionMayBeStatic
	size_t width() const { return WIDTH; }
	size_t height() const { return HEIGHT; }
	// ReSharper restore CppMemberFunctionMayBeStatic

	friend std::ostream &operator<<(std::ostream &os, const grid<T> &grid);
private:
	std::array<T, WIDTH * HEIGHT> grid_;
};

#else

template <typename T>
class grid
{
public:
	grid(const size_t w, const size_t h, const T &value) : grid_(0), width_(w), height_(h)
	{
		grid_.resize(w * h, value);
	}

	T &operator()(const size_t x, const size_t y) { return grid_[y * width_ + x]; }
	const T &operator()(const size_t x, const size_t y) const { return grid_[y * width_ + x]; }

	neighbors_t<T> neighbors(const size_t x, const size_t y) { return neighbors_t<T>(*this, x, y); }
	const_neighbors_t<T> neighbors(const size_t x, const size_t y) const { return const_neighbors_t<T>(*this, x, y); }

	size_t width() const { return width_; }
	size_t height() const { return height_; }

	template <typename U>
	friend std::ostream &operator<<(std::ostream &os, const grid<U> &grid);
private:
	std::vector<T> grid_;
	size_t width_, height_;
};

#endif

template <typename U>
std::ostream &operator<<(std::ostream &os, const grid<U> &grid)
{
	for (size_t y = 0; y < grid.height(); y++)
	{
		for (size_t x = 0; x < grid.width(); x++)
		{
			if (x != 0)
				os << " ";
			os << grid(x, y);
		}
		os << std::endl;
	}
	return os;
}
