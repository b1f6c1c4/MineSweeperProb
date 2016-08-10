// MineSweeperSpeed.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <string>
#include <fstream>
#include "../MineSweeperSolver/BinomialHelper.h"
#include "../MineSweeperSolver/GameMgr.h"
#include <iostream>

class PerformanceTimer
{
public:
    PerformanceTimer()
    {
        QueryPerformanceCounter(&m_StartTime);
    }

    void Restart()
    {
        QueryPerformanceCounter(&m_StartTime);
    }

    double Elapsed() const
    {
        LARGE_INTEGER end_time, frequency;
        QueryPerformanceCounter(&end_time);
        QueryPerformanceFrequency(&frequency);
        return double(end_time.QuadPart - m_StartTime.QuadPart)
            / frequency.QuadPart;
    }

private:
    LARGE_INTEGER m_StartTime;
};

std::pair<double, bool> ProcessID(const Strategy &strategy)
{
    GameMgr mgr(30, 16, 99);
    mgr.BasicStrategy = strategy;
    PerformanceTimer t;
    mgr.Automatic();
    return std::make_pair(t.Elapsed(), mgr.GetSucceed());
}

int main()
{
    CacheBinomials(30 * 16, 99);

    std::ifstream fin("config.txt");
    std::ofstream fout("benchmark.txt", std::ios::app);

    while (!fin.eof())
    {
        std::string str;
        int r;
        fin >> str >> r;
        if (str.length() == 0)
            continue;
        std::cout << str << " " << r << std::endl;
        auto strategy = ReadStrategy(str);
        while (r-- > 0)
        {
            auto result = ProcessID(strategy);
            fout << str << " ";
            fout.flags(std::ios::scientific);
            fout.precision(std::numeric_limits<double>::digits10 + 1);
            fout << result.second << " " << result.first << std::endl;
        }
        fout.flush();
    }

    fout.close();
    fin.close();

    return 0;
}
