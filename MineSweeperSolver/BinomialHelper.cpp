#include "BinomialHelper.h"
#include <vector>
#include <boost/thread/shared_mutex.hpp>
#include <boost/thread/lock_types.hpp>

static boost::shared_mutex mtx;
static std::vector<std::vector<double>> BinomialCoeff;

extern "C" void CacheBinomials(int n, int m)
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

    {
        boost::shared_lock<boost::shared_mutex> lock(mtx);
        boost::unique_lock<boost::shared_mutex> writeLock(mtx, boost::defer_lock);

#define UPGRADE \
    do { \
        if (lock.owns_lock()) \
        { \
            lock.unlock(); \
            lock.release(); \
            writeLock.lock(); \
        } \
    } while (false)

        if (BinomialCoeff.empty())
        {
            UPGRADE;
            if (BinomialCoeff.empty())
                BinomialCoeff.emplace_back(1, double(1));
        }

        if (BinomialCoeff.back().size() * 2 < m)
        {
            UPGRADE;
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
            UPGRADE;
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
}

double Binomial(int n, int m)
{
    boost::shared_lock<boost::shared_mutex> lock(mtx, boost::defer_lock);

    if (n < 0)
        return double(0);
    if (m > n ||
        m < 0)
        return double(0);
    if (m == 0 ||
        m == n)
        return double(1);

    auto mm = m <= n / 2 ? m : n - m;

    lock.lock();
    return BinomialCoeff[n - 1][mm - 1];
}
