#include "234.hpp"
#include <atomic>
#include <chrono>
#include <limits>
#include <thread>
#include <random>
#include <fmt/ranges.h>
#include <mimalloc-new-delete.h>

#define CRIT 1.542354e12

int main(int argc, char *argv[])
{
    auto n = std::atoi(argv[1]);
    auto m = (11451419 + n - 1) / n;
    Concurrent234<double, unsigned long long> tree;
    std::vector<std::thread> thr;
    std::atomic<long long> cnt;
    std::atomic<int> finished;
    for (auto i = 0; i < n; i++)
        thr.emplace_back([i,m,&tree,&cnt,&finished]{
            std::uniform_int_distribution<long long> dist(
                    1,
                    static_cast<long long>(0x6270A9FC70042000ull));
            std::mt19937_64 rnd;
            rnd.seed(i);
            auto lcnt = 0ll;
            for (auto j = 0zu; j < m; j++)
            {
                auto v = dist(rnd);
                auto d = *reinterpret_cast<double *>(&v);
                if (d > CRIT) lcnt++;
                v ^= 114514;
                auto res = tree.try_emplace(d, v);
                if (res.first != d)
                    throw std::logic_error{ "wrong key" };
                if (res.second != v)
                    throw std::logic_error{ "wrong payload" };
                if (lcnt >= 100)
                    cnt.fetch_add(lcnt, std::memory_order_relaxed), lcnt = 0;
            }
            cnt.fetch_add(lcnt, std::memory_order_relaxed), lcnt = 0;
            finished.fetch_add(1, std::memory_order_relaxed);
        });
    thr.emplace_back([&tree]{
        std::uniform_int_distribution<long long> dist(
                1,
                static_cast<long long>(0x6270A9FC70042000ull));
        std::mt19937_64 rnd;
        rnd.seed(114514);
        for (auto j = 0zu; j < 11451419; j++)
        {
            auto v = dist(rnd);
            auto d = *reinterpret_cast<double *>(&v);
            v ^= 114514;
            auto res = tree.find(d);
            if (!res)
                continue;
            if (res->first != d)
                throw std::logic_error{ "wrong key" };
            if (res->second != v)
                throw std::logic_error{ "wrong payload" };
        }
    });
    thr.emplace_back([&]{
        using namespace std::chrono_literals;
        auto last = 0ll;
        while (true) {
            std::this_thread::sleep_for(1s);
            if (finished.load(std::memory_order_relaxed) == n)
                break;
            auto c = cnt.load(std::memory_order_relaxed);
            auto d = static_cast<double>(c - last);
            fmt::print("#/s={:10.5e} #/t/s={:10.5e} h={} ##={:10.5e}\n", d, d / n, tree.get_height(), static_cast<double>(c));
            last = c;
        }
    });
    for (auto &th : thr)
        th.join();
    thr.clear();
    auto last = -std::numeric_limits<double>::infinity();
    fmt::println("Checking, cnt={}", cnt.load(std::memory_order_relaxed));
    tree.foreach(CRIT, [&](auto k, auto v){
        if (k <= CRIT)
            throw std::logic_error{ "not cutoff" };
        if (k <= last)
            throw std::logic_error{ "order incorrect" };
        auto d = *reinterpret_cast<long long *>(&k);
        auto x = d ^ v;
        if (x != 114514)
        {
            fmt::print("d={:16x} v={:16x} x={:16x}\n", d, v, x);
            throw std::logic_error{ "payload wrong" };
        }
        cnt.fetch_sub(1, std::memory_order_relaxed);
    });
    if (cnt.load(std::memory_order_relaxed))
        throw std::logic_error{ "cnt wrong" };
}
