#include "BinomialHelper.h"

static std::vector<std::vector<BigInteger>> BinomialCoeff;

void CacheBinomials(int n, int m)
{
    if (n < 0)
        return;
    if (m > n ||
        m < 0)
        return;
    if (m == 0)
        return;
    if (m > n / 2)
        m = n / 2;

    if (BinomialCoeff.back().size() * 2 < m)
        for (auto i = 0; i < BinomialCoeff.size(); ++i)
        {
            BinomialCoeff[i].reserve(min((i + 1) / 2, m + 1));
            for (auto j = BinomialCoeff[i].size(); j <= (i - 1) / 2 && j < m; j++)
                BinomialCoeff[i].push_back(
                                           BinomialCoeff[i - 1][j - 1] +
                                           (j == (i - 1) / 2 && i % 2 == 1 ? BinomialCoeff[i - 1][j - 1] : BinomialCoeff[i - 1][j]));
        }
    BinomialCoeff.reserve(n);
    for (auto i = BinomialCoeff.size(); i < n; i++)
    {
        auto lst = std::vector<BigInteger>();
        lst.reserve(min((i - 1) / 2 + 1, m));
        lst.push_back(1 + BinomialCoeff[i - 1][0]);
        for (auto j = 1; j <= (i - 1) / 2 && j < m; j++)
            lst.push_back(
                          BinomialCoeff[i - 1][j - 1] +
                          (j == (i - 1) / 2 && i % 2 == 1 ? BinomialCoeff[i - 1][j - 1] : BinomialCoeff[i - 1][j]));
        BinomialCoeff.push_back(lst);
    }
}

BigInteger Binomial(int n, int m)
{
    if (n < 0)
        return BigInteger(0);
    if (m > n ||
        m < 0)
        return BigInteger(0);
    if (m == 0 ||
        m == n)
        return BigInteger(1);

    auto mm = m <= n / 2 ? m : n - m;

    return BinomialCoeff[n - 1][mm - 1];
}
