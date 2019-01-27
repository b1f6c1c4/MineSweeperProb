#include "lp.h"
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
	lp_impl(const std::list<area> &areas, size_t sz, size_t cons);
	lp_impl(const lp_impl &other);
	lp_impl(lp_impl &&other) noexcept;
	~lp_impl();

	lp_impl &operator=(const lp_impl &other);
	lp_impl &operator=(lp_impl &&other) noexcept;

	void reset(size_t sz, size_t cons) override;

	void constraint(const std::vector<area*> &neighbors, size_t m) override;
	void constraint(size_t m) override;

	logic_result solve() override;

	const std::vector<lp_result_t> &get_result() const override;

private:
	lprec *lp_;
	std::vector<lp_result_t> results_;
	size_t rowid_;

	size_t solve(size_t index, int dir);
	void row_eq(const std::vector<REAL> &row, const std::vector<int> &colno, size_t m);
};

lp_impl::lp_impl(const std::list<area> &areas, const size_t sz, const size_t cons)
	: lp<area, list_simple>(areas, sz), lp_(nullptr), rowid_(0)
{
	lp_ = make_lp(static_cast<int>(cons), static_cast<int>(sz));
	if (!lp_)
		throw;

	lp_impl::reset(sz, cons);
}

lp_impl::lp_impl(const lp_impl &other) : lp<area, list_simple>(other), lp_(nullptr), rowid_(other.rowid_)
{
	if (other.lp_)
	{
		lp_ = copy_lp(other.lp_);
		if (!lp_)
			throw;
	}
}

lp_impl::lp_impl(lp_impl &&other) noexcept : lp<area, list_simple>(std::move(other)), lp_(other.lp_),
                                             rowid_(other.rowid_)
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
	rowid_ = other.rowid_;
	return *this;
}

lp_impl &lp_impl::operator=(lp_impl &&other) noexcept
{
	lp_ = other.lp_;
	if (&other != this)
		other.lp_ = nullptr;
	rowid_ = other.rowid_;

	lp<area, list_simple>::operator=(std::move(other));
	return *this;
}

void lp_impl::reset(const size_t sz, const size_t cons)
{
	resize_lp(lp_, static_cast<int>(cons), static_cast<int>(sz));
	while (get_Ncolumns(lp_) < static_cast<int>(sz))
	{
		auto col{0.0};
		auto rowno{0};
		if (!add_columnex(lp_, 0, &col, &rowno))
			throw;
	}
	while (get_Nrows(lp_) < static_cast<int>(cons))
	{
		auto col{ 0.0 };
		auto rowno{ 0 };
		if (!add_constraintex(lp_, 0, &col, &rowno, LE, -1))
			throw;
	}

	size_t id = 0;
	for (auto &a : areas_.get())
	{
		set_int(lp_, static_cast<int>(id + 1), TRUE);
		set_bounds(lp_, static_cast<int>(id + 1), 0, static_cast<double>(a.size()));
		id++;
	}

	ncols_ = sz;
	rowid_ = 0;
}

void lp_impl::constraint(const std::vector<area *> &neighbors, const size_t m)
{
	const std::vector<REAL> row(neighbors.size(), 1);
	std::vector<int> colno;
	colno.reserve(neighbors.size());
	for (auto a : neighbors)
		colno.push_back(static_cast<int>(a->index + 1));
	row_eq(row, colno, m);
}

void lp_impl::constraint(const size_t m)
{
	const std::vector<REAL> row(ncols_, 1);
	std::vector<int> colno;
	colno.reserve(ncols_);
	for (size_t i = 0; i < ncols_; i++)
		colno.push_back(static_cast<int>(i + 1));
	row_eq(row, colno, m);
}

logic_result lp_impl::solve()
{
#ifndef NDEBUG
	// write_LP(lp_, stdout);
	set_verbose(lp_, IMPORTANT);
#else
	set_verbose(lp_, NEUTRAL);
#endif

	std::vector<REAL> row{1};
	std::vector<int> colno(1);

	auto flag = false;
	results_.clear();
	for (auto &it : areas_.get())
	{
		const auto lb = solve(it.index, +1);
		if (lb < 0 || lb > it.size())
			return logic_result::invalid;

		const auto ub = solve(it.index, -1);
		if (ub < 0 || ub > it.size())
			return logic_result::invalid;

		if (lb == it.size() || ub == 0)
			flag = true;
		results_.emplace_back(std::make_pair<size_t, size_t>(static_cast<size_t>(lb), static_cast<size_t>(ub)));
	}
	return flag ? logic_result::dirty : logic_result::clean;
}

size_t lp_impl::solve(const size_t index, const int dir)
{
	auto row = static_cast<double>(dir);
	auto colno = static_cast<int>(index + 1);

	if (!set_obj_fnex(lp_, 1, &row, &colno))
		throw;

	const auto res = ::solve(lp_);
	if (res == INFEASIBLE)
		return -1;
	if (res != OPTIMAL)
		throw;

	return static_cast<size_t>(dir * get_objective(lp_));
}

void lp_impl::row_eq(const std::vector<double> &row, const std::vector<int> &colno, const size_t m)
{
	++rowid_;

	if (!set_rowex(lp_, static_cast<int>(rowid_), static_cast<int>(row.size()),
	               const_cast<double*>(&*row.begin()), const_cast<int*>(&*colno.begin())))
		throw;
	if (!set_constr_type(lp_, static_cast<int>(rowid_), EQ))
		throw;
	if (!set_rh(lp_, static_cast<int>(rowid_), static_cast<double>(m)))
		throw;
}

const std::vector<lp_result_t> &lp_impl::get_result() const
{
	return results_;
}

std::shared_ptr<lp<area, list_simple>> make_lp(const std::list<area> &areas, const size_t sz, const size_t cons)
{
	return std::make_shared<lp_impl>(areas, sz, cons);
}
