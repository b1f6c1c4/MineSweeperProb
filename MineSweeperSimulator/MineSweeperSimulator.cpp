#include "stdafx.h"
#include <iostream>
#include "../MineSweeperSolver/GameMgr.h"
#include "../MineSweeperSolver/BinomialHelper.h"
#include "ThreadPool.h"

int main()
{
    CacheBinomials(30 * 16, 99);
    
    int thrs, rpN, rpM;
    std::cout << "ThreadPool pool(?): ";
    std::cin >> thrs;
    std::cout << "M tasks: ";
    std::cin >> rpM;
    std::cout << "N games: ";
    std::cin >> rpN;

    std::mutex mtx;
    auto succeed = 0, total = 0;

    {
        ThreadPool pool(thrs);
        for (auto i = 0; i < rpM; ++i)
            pool.enqueue([&succeed, &total, &mtx, rpN]()
            {
                auto t = 0, s = 0;
                for (auto n = 0; n < rpN; ++n)
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

        while (total < rpM * rpN)
        {
            std::this_thread::sleep_for(std::chrono::seconds{ 1 });
            std::cout << succeed << "/" << total << "=" << static_cast<double>(succeed) / total << std::endl;
        }
    }


    system("pause");
    return 0;
}
