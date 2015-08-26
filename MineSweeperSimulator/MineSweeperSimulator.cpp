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
volatile int rest;
volatile auto succeed = 0, total = 0;

void Process()
{
    while (true)
    {
        {
            std::unique_lock<std::mutex> lock(mtx);
            if (rest == 0)
                break;
            --rest;
        }

        auto mgr = GameMgr(30, 16, 99);
        mgr.Automatic();

        {
            std::unique_lock<std::mutex> lock(mtx);
            if (mgr.GetSucceed())
                ++succeed;
            ++total;
        }
    }
}

int main()
{
    CacheBinomials(30 * 16, 99);

    int thrs, rpM, wait;
    std::ifstream fin("config.txt");
    fin >> thrs >> wait >> rpM;
    fin.close();

    std::cout << thrs << " " << wait << " " << rpM << std::endl;

    rest = rpM;

    std::ofstream fout("output.txt", std::ios::app);
        
    auto succeedT = 0, totalT = 0;

    while (thrs-- > 0)
        m_Threads.emplace_back(&Process);

    auto i = 0;
    while (rest > 0)
    {
        std::this_thread::sleep_for(std::chrono::seconds{ wait });

        i += wait;
        mtx.lock();
        if (total > 0)
            fout << succeed << " " << total << std::endl << std::flush;
        succeedT += succeed;
        totalT += total;
        succeed = 0;
        total = 0;
        mtx.unlock();

        auto est = static_cast<int>(totalT == 0 ? 0 : static_cast<double>(i) * rpM / totalT - i);
        auto estMin = est / 60 % 60;
        auto estHour = est / 60 / 60;
        std::cout << succeedT << " / " << totalT << " " << estHour << ":" << estMin << ":" << est << " " << static_cast<double>(succeedT) / (totalT) << std::endl;
        if (est < wait)
            break;
    }

    for (auto &th : m_Threads)
        th.join();

    if (total > 0)
        fout << succeed << " " << total << std::endl << std::flush;
    fout.close();

    succeedT += succeed;
    totalT += total;
    std::cout << succeedT << " / " << totalT << " " << static_cast<double>(succeedT) / (totalT) << std::endl;

    system("pause");
    return 0;
}
