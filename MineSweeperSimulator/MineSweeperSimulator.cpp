#include "stdafx.h"
#include <iomanip>
#include <iostream>
#include <fstream>
#include "../MineSweeperSolver/GameMgr.h"
#include "../MineSweeperSolver/BinomialHelper.h"
#include "ThreadPool.h"

int main()
{
    CacheBinomials(30 * 16, 99);

    int thrs, rpM;
    std::ifstream fin("config.txt");
    fin >> thrs >> rpM;
    fin.close();

    std::cout << thrs << " " << rpM << std::endl;

    std::ofstream fout("output.txt", std::ios::app);
        
    std::mutex mtx;
    auto succeedT = 0, totalT = 0;
    volatile auto succeed = 0, total = 0;

    {
        ThreadPool pool(thrs);
        for (auto i = 0; i < rpM; ++i)
            pool.enqueue([&succeed, &total, &mtx]()
                         {
                             auto t = 0, s = 0;
                             //for (auto n = 0; n < rpN; ++n)
                             {
                                 auto mgr = GameMgr(30, 16, 99);
                                 mgr.Automatic();
                                 ++t;
                                 if (mgr.GetSucceed())
                                     ++s;
                             }
                             mtx.lock();
                             succeed += s;
                             total += t;
                             mtx.unlock();
                         });

        auto i = 0;
        while (totalT + total < rpM)
        {
            std::this_thread::sleep_for(std::chrono::seconds{1});

            if (++i > 60)
            {
                i = 0;
                mtx.lock();
                if (total > 0)
                    fout << succeed << " " << total << std::endl << std::flush;
                succeedT += succeed;
                totalT += total;
                succeed = 0;
                total = 0;
                mtx.unlock();
            }

            auto est = totalT + total == 0 ? 0 : i * rpM / (totalT + total) - i;
            auto estMin = est / 60 % 60;
            auto estHour = est / 60 / 60;
            std::cout << succeedT + succeed << " / " << totalT + total << " " << estHour << ":" << estMin << ":" << est << " " << static_cast<double>(succeedT + succeed) / (totalT + total) << std::endl;
        }
    }
    fout << succeed << " " << total << std::endl << std::flush;
    fout.close();

    system("pause");
    return 0;
}
