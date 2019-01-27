#pragma once
#include "common.h"
#include <list>
#include <functional>
#include "base_logic.h"

typedef std::pair<size_t, size_t> lp_result_t;

template <typename T, template <typename> class V>
class lp
{
public:
	virtual ~lp();

	lp<T, V> &operator=(const lp<T, V> &other);
	lp<T, V> &operator=(lp<T, V> &&other) noexcept;

	virtual void constraints(const std::vector<T*> &neighbors, size_t m) = 0;

	virtual logic_result solve() = 0;

	virtual const std::vector<lp_result_t> &get_result() const = 0;

protected:
	explicit lp(const V<T> &areas);
	lp(const lp<T, V> &other);
	lp(lp<T, V> &&other) noexcept;

	std::reference_wrapper<const V<T>> areas_;
};

template <typename T, template <typename> class V>
lp<T, V>::lp(const V<T> &areas) : areas_(areas) { }

template <typename T, template <typename> class V>
lp<T, V>::lp(const lp<T, V> &other) : areas_(other.areas_) { }

template <typename T, template <typename> class V>
lp<T, V>::lp(lp<T, V> &&other) noexcept : areas_(other.areas_) { }

template <typename T, template <typename> class V>
lp<T, V>::~lp() = default;

template <typename T, template <typename> class V>
lp<T, V> &lp<T, V>::operator=(const lp<T, V> &other)
{
	areas_ = other.areas_;
	return *this;
}

template <typename T, template <typename> class V>
lp<T, V> &lp<T, V>::operator=(lp<T, V> &&other) noexcept
{
	areas_ = other.areas_;
	return *this;
}

template <typename T>
using list_simple = std::list<T>;

struct area;
std::shared_ptr<lp<area, list_simple>> make_lp(const std::list<area> &areas, size_t sz);
