#include "stdafx.h"
#include <iomanip>
#include <iostream>
#include <fstream>
#include <thread>
#include <mutex>
#include "../MineSweeperSolver/GameMgr.h"
#include "../MineSweeperSolver/BinomialHelper.h"

std::mutex mtx;
std::vector<std::thread> m_Threads;

int numTasks;
size_t *cert;
size_t *dense;
volatile size_t restT, totalTT;
volatile size_t *rest;
volatile size_t *succeed, *succeedT;
volatile size_t *total, *totalT;

bool ProcessID(int id)
{
    auto mgr = GameMgr(30, 16, 99);
    mgr.DrainCriterion = cert[id];
    mgr.OpenBlock(0, 0);
    mgr.Automatic();
    return mgr.GetSucceed();
}

void Process(int id, bool (*fun)(int))
{
    for (;;++id)
    {
        id %= numTasks;
        auto d = dense[id];
        while (d-- > 0)
        {
            {
                std::unique_lock<std::mutex> lock(mtx);
                if (restT == 0)
                    break;

                if (rest[id] == 0)
                    continue;

                --restT;
                --rest[id];
            }

            auto res = fun(id);

            {
                std::unique_lock<std::mutex> lock(mtx);
                if (res)
                    ++succeed[id];
                ++total[id];
            }
        }
    }
}

int main()
{
    CacheBinomials(30 * 16, 99);

    int thrs, wait;
    std::ifstream fin("config.txt");
    fin >> thrs >> wait >> numTasks;
#undef max
    auto min = std::numeric_limits<size_t>().max();
    totalTT = restT = 0;
    dense = new size_t[numTasks];
    cert = new size_t[numTasks];
    rest = new size_t[numTasks];
    succeed = new size_t[numTasks];
    total = new size_t[numTasks];
    succeedT = new size_t[numTasks];
    totalT = new size_t[numTasks];
    for (auto i = 0; i < numTasks; ++i)
    {
        int r;
        fin >> cert[i] >> r;
        dense[i] = r;
        if (dense[i] < min)
            min = dense[i];
        rest[i] = r;
        restT += r;
        succeedT[i] = succeed[i] = 0;
        totalT[i] = total[i] = 0;
    }
    fin.close();

    size_t cnt = 0;
    while (min != 0)
    {
        min >>= 1;
        ++cnt;
    }
    --cnt;
    for (auto i = 0; i < numTasks; ++i)
        dense[i] >>= cnt;

    std::cout << thrs << " " << wait << std::endl;

    std::ofstream fout("output.txt", std::ios::app);

    while (thrs-- > 0)
        m_Threads.emplace_back(&Process, thrs, &ProcessID);

    auto last = 0;
    while (true)
    {
        std::this_thread::sleep_for(std::chrono::seconds{wait});

        last += wait;
        size_t rT;
        {
            std::unique_lock<std::mutex> lock(mtx);

            for (auto i = 0; i < numTasks;++i)
            {
                if (total[i] == 0)
                    continue;
                
                fout << cert[i] << " " << succeed[i] << " " << total[i] << std::endl << std::flush;
                succeedT[i] += succeed[i];
                totalT[i] += total[i];
                totalTT += total[i];
                succeed[i] = 0;
                total[i] = 0;
            }
            rT = restT;
        }

        auto est = static_cast<int>(totalTT == 0 ? 0 : static_cast<float>(last) / totalTT * rT);
        auto estMin = est / 60 % 60;
        auto estHour = est / 60 / 60;

        std::cout << last << " " << estHour << ":" << estMin << ":" << est << " " << static_cast<double>(totalTT) / (totalTT + rT) * 100 << "%" << std::endl;
        for (auto i = 0; i < numTasks; ++i)
            std::cout << cert[i] << ": " << succeedT[i] << " / " << totalT[i] << "=" << static_cast<double>(succeedT[i]) / totalT[i] << std::endl;
        std::cout << std::endl;

        if (rT == 0)
            break;
    }

    for (auto &th : m_Threads)
        th.join();

    for (auto i = 0; i < numTasks; ++i)
    {
        if (total[i] == 0)
            continue;

        fout << cert[i] << " " << succeed[i] << " " << total[i] << std::endl << std::flush;
        succeedT[i] += succeed[i];
        totalT[i] += total[i];
        totalTT += total[i];
        succeed[i] = 0;
        total[i] = 0;
    }

    std::cout << last << " -----" << std::endl;
    for (auto i = 0; i < numTasks; ++i)
        std::cout << cert[i] << ": " << succeedT[i] << " / " << totalT[i] << "=" << static_cast<double>(succeedT[i]) / totalT[i] << std::endl;
    std::cout << std::endl;

    system("pause");
    return 0;
}
