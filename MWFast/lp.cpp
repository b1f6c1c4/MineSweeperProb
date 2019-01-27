#include "lp.h"
#include <iostream>
#include "full_logic.h"

extern "C" {
#undef _WIN32
#undef WIN32
#undef _WIN64
#undef WIN64
#define LoadInverseLib FALSE
#define LoadLanguageLib FALSE
#include <lp_lib.h>

// Fix bullshit code from https://stackoverflow.com/a/32449318

FILE _iob[] = {*stdin, *stdout, *stderr}; // NOLINT

// ReSharper disable once CppRedundantVoidArgumentList
FILE *__cdecl __iob_func(void)
{
	return _iob;
}
}

// end of fix

class lp_impl : public lp<area, list_simple>
{
public:
	lp_impl(const std::list<area> &areas, size_t sz);
	lp_impl(const lp_impl &other);
	lp_impl(lp_impl &&other) noexcept;
	~lp_impl();

	lp_impl &operator=(const lp_impl &other);
	lp_impl &operator=(lp_impl &&other) noexcept;

	void constraints(const std::vector<area*> &neighbors, size_t m) override;

	logic_result solve() override;

	const std::vector<lp_result_t> &get_result() const override;

private:
	lprec *lp_;
	std::vector<lp_result_t> results_;

	size_t solve(size_t index, int dir);
};

lp_impl::lp_impl(const std::list<area> &areas, const size_t sz) : lp<area, list_simple>(areas), lp_(nullptr)
{
	lp_ = make_lp(0, static_cast<int>(sz));
	if (!lp_)
		throw;

	size_t id = 0;
	for (auto it = areas.begin(); it != areas.end(); ++it)
	{
		set_int(lp_, static_cast<int>(id + 1), TRUE);
		set_bounds(lp_, static_cast<int>(id + 1), 0, static_cast<double>(it->size()));
		id++;
	}
}

lp_impl::lp_impl(const lp_impl &other) : lp<area, list_simple>(other), lp_(nullptr)
{
	if (other.lp_)
	{
		lp_ = copy_lp(other.lp_);
		if (!lp_)
			throw;
	}
}

lp_impl::lp_impl(lp_impl &&other) noexcept : lp<area, list_simple>(std::move(other)), lp_(other.lp_)
{
	if (&other != this)
		other.lp_ = nullptr;
}

lp_impl::~lp_impl()
{
	if (lp_ != nullptr)
	{
		delete_lp(lp_);
		lp_ = nullptr;
	}
}

lp_impl &lp_impl::operator=(const lp_impl &other)
{
	lp<area, list_simple>::operator=(other);

	if (other.lp_)
	{
		lp_ = copy_lp(other.lp_);
		if (!lp_)
			throw;
	}
	else
		lp_ = nullptr;
	return *this;
}

lp_impl &lp_impl::operator=(lp_impl &&other) noexcept
{
	lp_ = other.lp_;
	if (&other != this)
		other.lp_ = nullptr;

	lp<area, list_simple>::operator=(std::move(other));
	return *this;
}

/*void lp_impl::have_fun()
{
	lp_ = make_lp(0, 2);
	if (!lp_)
		throw;

	set_col_name(lp_, 1, const_cast<char *>("x"));
	set_col_name(lp_, 2, const_cast<char *>("y"));

	set_int(lp_, 1, FALSE);
	set_int(lp_, 2, TRUE);

	//set_bounds(lp_, 1, FALSE);
	set_int(lp_, 2, TRUE);

	set_add_rowmode(lp_, TRUE);

	{
		std::vector<REAL> row{ 120, 210 };
		std::vector<int> colno{ 1, 2 };

		if (!add_constraintex(lp_, 2, &*row.begin(), &*colno.begin(), LE, 15000))
			throw;
	}
	{
		std::vector<REAL> row{ 110, 30 };
		std::vector<int> colno{ 1, 2 };

		if (!add_constraintex(lp_, 2, &*row.begin(), &*colno.begin(), LE, 4000))
			throw;
	}
	{
		std::vector<REAL> row{ 1, 1 };
		std::vector<int> colno{ 1, 2 };

		if (!add_constraintex(lp_, 2, &*row.begin(), &*colno.begin(), LE, 75))
			throw;
	}

	set_add_rowmode(lp_, FALSE);

	{
		std::vector<REAL> row{ 143, 60 };
		std::vector<int> colno{ 1, 2 };

		if (!set_obj_fnex(lp_, 2, &*row.begin(), &*colno.begin()))
			throw;
		set_maxim(lp_);
	}

	write_LP(lp_, stdout);
	set_verbose(lp_, IMPORTANT);
	const auto res = solve(lp_);
	if (res == OPTIMAL)
	{
		std::cout << "Objective value: " << get_objective(lp_) << std::endl;

		REAL *ptr;
		get_ptr_variables(lp_, &ptr);
		for (auto j = 0; j < 2; j++)
			std::cout << ptr[j] << std::endl;
	}
	else
		std::cout << "The fuck?" << res << std::endl;
}*/

void lp_impl::constraints(const std::vector<area *> &neighbors, const size_t m)
{
	set_add_rowmode(lp_, TRUE);

	std::vector<REAL> row(neighbors.size(), 1);
	std::vector<int> colno;
	colno.reserve(neighbors.size());
	for (auto a : neighbors)
		colno.push_back(static_cast<int>(a->index + 1));

	if (!add_constraintex(lp_, static_cast<int>(neighbors.size()), &*row.begin(), &*colno.begin(), EQ,
	                      static_cast<double>(m)))
		throw;
}

logic_result lp_impl::solve()
{
	set_add_rowmode(lp_, FALSE);
	set_verbose(lp_, IMPORTANT);

	std::vector<REAL> row{1};
	std::vector<int> colno(1);

	auto flag = false;
	results_.clear();
	for (auto &it : areas_.get())
	{
		const auto lb = solve(it.index, 1);
		if (lb < 0 || lb > it.size())
			return logic_result::invalid;
		if (lb > 0)
			flag = true;

		const auto ub = solve(it.index, 1);
		if (ub < 0 || ub > it.size())
			return logic_result::invalid;
		if (ub < it.size())
			flag = true;

		results_.emplace_back(std::make_pair<size_t, size_t>(static_cast<size_t>(lb), static_cast<size_t>(ub)));
	}
	return flag ? logic_result::dirty : logic_result::clean;
}

size_t lp_impl::solve(const size_t index, const int dir)
{
	auto row = static_cast<double>(dir);
	auto colno = static_cast<int>(index);

	if (!set_obj_fnex(lp_, 1, &row, &colno))
		throw;

	const auto res = ::solve(lp_);
	if (res == INFEASIBLE)
		return -1;
	if (res != OPTIMAL)
		throw;

	return static_cast<size_t>(get_objective(lp_));
}

const std::vector<lp_result_t> &lp_impl::get_result() const
{
	return results_;
}

std::shared_ptr<lp<area, list_simple>> make_lp(const std::list<area> &areas, size_t sz)
{
	return std::make_shared<lp_impl>(areas, sz);
}
