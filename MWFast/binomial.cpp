#include "binomial.h"

rep_t binomial(const size_t n, const size_t m)
{
	rep_t res = 1;
	for (auto i = m + 1; i <= n; i++)
	{
		res *= i;
		res /= i - m;
	}
	return res;
}
