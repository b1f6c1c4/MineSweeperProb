#include "stdafx.h"
#include "../MineSweeperSolver/GameMgr.h"
#include <iostream>
#include "../MineSweeperSolver/BinomialHelper.h"

int main()
{
    CacheBinomials(30 * 16, 99);
    while (true)
    {
        auto mgr = GameMgr(30, 16, 99);
        mgr.Automatic();
        std::cout << mgr.GetSucceed() << ":" << mgr.GetBits() << std::endl;
    }
    system("pause");
    return 0;
}
