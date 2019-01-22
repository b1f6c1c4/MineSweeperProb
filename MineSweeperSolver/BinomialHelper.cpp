#include "BinomialHelper.h"
#include <vector>

static std::vector<std::vector<double>> BinomialCoeff;

void CacheBinomials(int n, int m)
{
    ++n , ++m;
    if (n < 0)
        return;
    if (m < 0)
        return;
    if (m == 0)
        return;
    if (m > n / 2)
        m = n / 2;

    if (BinomialCoeff.empty())
    {
        if (BinomialCoeff.empty())
            BinomialCoeff.emplace_back(1, double(1));
    }

    if (BinomialCoeff.back().size() * 2 < m)
    {
        for (auto i = 0; i < BinomialCoeff.size(); ++i)
        {
            auto &lst = BinomialCoeff[i];
            lst.reserve(MIN((i + 1) / 2, m + 1));
            for (auto j = lst.size(); j <= (i - 1) / 2 && j < m; ++j)
            {
                lst.emplace_back(BinomialCoeff[i - 1][j - 1]);
                lst.back() += j == (i - 1) / 2 && i % 2 == 1 ? BinomialCoeff[i - 1][j - 1] : BinomialCoeff[i - 1][j];
            }
        }
    }

    if (BinomialCoeff.size() < n)
    {
        BinomialCoeff.reserve(n);
        for (auto i = BinomialCoeff.size(); i < n; ++i)
        {
            BinomialCoeff.emplace_back();
            auto &lst = BinomialCoeff.back();
            lst.reserve(MIN((i - 1) / 2 + 1, m));
            lst.push_back(1 + BinomialCoeff[i - 1][0]);
            for (auto j = 1; j <= (i - 1) / 2 && j < m; ++j)
            {
                lst.emplace_back(BinomialCoeff[i - 1][j - 1]);
                lst.back() += j == (i - 1) / 2 && i % 2 == 1 ? BinomialCoeff[i - 1][j - 1] : BinomialCoeff[i - 1][j];
            }
        }
    }
}

double Binomial(int n, int m)
{
    if (n < 0)
        return double(0);
    if (m > n ||
            m < 0)
        return double(0);
    if (m == 0 ||
            m == n)
        return double(1);

    auto mm = m <= n / 2 ? m : n - m;

    return BinomialCoeff[n - 1][mm - 1];
}
