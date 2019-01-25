#include "lp.h"

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

FILE *__cdecl __iob_func(void)
{
	return _iob;
}

// end of fix
}

class lp_impl : public lp
{
public:
	lp_impl();
	lp_impl(const lp_impl &other);
	lp_impl(lp_impl &&other) noexcept;
	~lp_impl();

	lp_impl &operator=(const lp_impl &other);
	lp_impl &operator=(lp_impl &&other) noexcept;

private:
	lprec *lp_;
};


lp_impl::lp_impl() : lp_(make_lp(0, 0)) {}

lp_impl::lp_impl(const lp_impl &other) : lp_(copy_lp(other.lp_)) { }

lp_impl::lp_impl(lp_impl &&other) noexcept : lp_(other.lp_)
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
	lp_ = copy_lp(other.lp_);
	return *this;
}

lp_impl &lp_impl::operator=(lp_impl &&other) noexcept
{
	lp_ = other.lp_;
	if (&other != this)
		other.lp_ = nullptr;
	return *this;
}
