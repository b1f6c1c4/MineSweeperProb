// MineSweeperProver.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <windows.h>
#include <psapi.h>
#include <vector>
#include <deque>
#include <unordered_map>
#include "../MineSweeperSolver/GameMgr.h"
#include "../MineSweeperSolver/BinomialHelper.h"
#include "../MineSweeperSolver/Solver.h"
#include <unordered_set>
#include <iostream>

size_t MemoryLimit;

bool IsMemoryEnough()
{
    PROCESS_MEMORY_COUNTERS pmc;
    GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc));
    auto mem = pmc.WorkingSetSize;
    return mem < MemoryLimit;
}

int Width, Height, Mines;
std::vector<BlockSet> m_BlocksR;

int GetIndex(int x, int y)
{
    return x * Height + y;
}

struct MacroSituation;
struct Choice
{
    explicit Choice(int blk) : Block(blk), m_UpperBound(NAN) { }
    int Block;
    std::unordered_map<MacroSituation *, double> Next;
    double m_UpperBound;
};

struct MacroSituation
{
    MacroSituation() : m_Depth(0), m_Solver(nullptr), m_UpperBound(NAN) { }
    MacroSituation(const MacroSituation &other) : m_Depth(other.m_Depth + 1), m_Degrees(other.m_Degrees), m_Solver(nullptr), m_UpperBound(NAN)
    {
        if (other.m_Solver != nullptr)
            m_Solver = new Solver(*other.m_Solver);
    }

#define CLOSED static_cast<unsigned char>(0xff)
    int m_Depth;
    std::vector<unsigned char> m_Degrees;
    Solver *m_Solver;
    std::vector<Choice> m_Choices;
    double m_UpperBound;
    BlockSet m_BestChoices;
};

struct Hash
{
    size_t operator() (MacroSituation * const &macro) const
    {
        size_t h = 5381;
        for (auto v : macro->m_Degrees)
            h = (h << 5) + h + v;
        return h;
    }
};
bool operator==(const MacroSituation &lhs, const MacroSituation &rhs)
{
    return lhs.m_Degrees == rhs.m_Degrees;
}
bool operator!=(const MacroSituation &lhs, const MacroSituation &rhs)
{
    return !(lhs == rhs);
}

std::deque<MacroSituation *> WorkingQueue;
std::vector<std::unordered_set<MacroSituation *, Hash>> Situations;
std::vector<size_t> Queued;

void OpenBlock(MacroSituation *macro, Block blk, bool forceNoMine = false)
{
    if (macro->m_Degrees[blk] != CLOSED)
        return;
    macro->m_Solver->Solve(SolvingState::Reduce | SolvingState::Overlap | SolvingState::Probability, false);
    int min;
    auto &info = macro->m_Solver->GetDistInfo(m_BlocksR[blk], blk, min);
    
    macro->m_Choices.emplace_back(blk);
    for (auto i = 0; i <= info.Length; ++i)
    {
        if (info.m_States[i] == 0)
            continue;

        auto child = new MacroSituation(*macro);
        child->m_Degrees[blk] = min + i;
        if (child->m_Depth >= Situations.size())
        {
            Situations.emplace_back();
            Queued.push_back(0);
        }
        auto res = Situations[child->m_Depth].insert(child);
        if (res.second)
        {
            child->m_Solver->AddRestrain(blk, false);
            if (child->m_Solver->GetBlockStatus(blk) == BlockStatus::Blank)
                --child->m_Solver->CanOpenForSure;
            child->m_Solver->AddRestrain(m_BlocksR[blk], min + i);
            ++Queued[child->m_Depth];
        }
        else
        {
            delete child;
            child = *res.first;
        }
        if (forceNoMine)
            macro->m_Choices.back().Next.insert(std::make_pair(child, info.m_Result[i]));
        else
            macro->m_Choices.back().Next.insert(std::make_pair(child, info.m_Result[i]
                * (1 - macro->m_Solver->GetProbability(blk))));
        WorkingQueue.push_back(child);
    }
}

void GetUpperBound(MacroSituation *macro, int maxDepth)
{
    if (macro->m_Solver != nullptr)
    {
        delete macro->m_Solver;
        macro->m_Solver = nullptr;
    }

    if (macro->m_Depth == maxDepth)
        return;

    double upper = 0;
    for (auto &choice : macro->m_Choices)
    {
        double b = 0;
        for (auto &kvp : choice.Next)
        {
            GetUpperBound(kvp.first, maxDepth);
            b += kvp.first->m_UpperBound * kvp.second;
        }
        if (b > upper)
        {
            upper = b;
            macro->m_BestChoices.clear();
            macro->m_BestChoices.push_back(choice.Block);
        }
        else if (b >= upper - upper * 1E-8)
            macro->m_BestChoices.push_back(choice.Block);
    }

    macro->m_UpperBound = upper;
    std::vector<Choice>().swap(macro->m_Choices);
}

int main()
{
    std::cout << "Width Height Mines: ";
    std::cin >> Width >> Height >> Mines;

    CacheBinomials(Width * Height, Mines);

    Situations.resize(10);
    Queued.resize(10);

    std::cout << "Max Memory Usage (MB): ";
    std::cin >> MemoryLimit;
    MemoryLimit <<= 20;

    m_BlocksR.reserve(Width * Height);
    for (auto i = 0; i < Width; ++i)
        for (auto j = 0; j < Height; ++j)
        {
            m_BlocksR.emplace_back();
            auto &blkR = m_BlocksR.back();
            blkR.reserve(8);
            for (auto di = -1; di <= 1; ++di)
                if (i + di >= 0 && i + di < Width)
                    for (auto dj = -1; dj <= 1; ++dj)
                        if (j + dj >= 0 && j + dj < Height)
                            if (di != 0 || dj != 0)
                                blkR.push_back(GetIndex(i + di, j + dj));
        }

    auto root = new MacroSituation;
    root->m_Degrees.resize(Width*Height, CLOSED);
    root->m_Solver = new Solver(Width * Height, Mines);
    root->m_Solver->Solve(SolvingState::Reduce | SolvingState::Overlap | SolvingState::Probability, false);
    Situations[0].insert(root);
    OpenBlock(root, 0, true);
    //WorkingQueue.push_back(root);

    std::cout << "Forking..." << std::endl;

    auto maxDepth = 0;
    BlockSet preferred;
    auto prun = false;
    while (!WorkingQueue.empty())
    {
        if (!prun && !IsMemoryEnough())
        {
            std::cout << "Pruning..." << std::endl;
            prun = true;
        }

        auto macro = WorkingQueue.front();
        WorkingQueue.pop_front();
        if (prun && macro->m_Depth > maxDepth)
        {
            delete macro;
            continue;
        }
        --Queued[macro->m_Depth];
        macro->m_Solver->Solve(SolvingState::Reduce | SolvingState::Overlap | SolvingState::Probability, false);
        if (macro->m_Solver->CanOpenForSure > 0)
        {
            macro->m_UpperBound = 1;
            if (!prun && macro->m_Solver->GetTotalStates() > 1)
            for (auto i = 0; i < Width * Height; ++i)
                if (macro->m_Degrees[i] == CLOSED && macro->m_Solver->GetBlockStatus(i) == BlockStatus::Blank)
                {
                    OpenBlock(macro, i);
                    break;
                }
        }
        else
        {
            for (auto i = 0; i < Width * Height; ++i)
                if (macro->m_Degrees[i] == CLOSED && macro->m_Solver->GetBlockStatus(i) == BlockStatus::Unknown)
                    macro->m_BestChoices.push_back(i);

#define LARGEST(exp) Largest(macro->m_BestChoices, std::function<double(Block)>([macro](Block blk) { return exp; } ))
            LARGEST(macro->m_Solver->UpperBoundCondQ(m_BlocksR[blk], blk)*(1 - macro->m_Solver->GetProbability(macro->m_BestChoices.front())));
            int min;
            macro->m_UpperBound = macro->m_Solver->GetDistInfo(m_BlocksR[macro->m_BestChoices.front()], macro->m_BestChoices.front(), min).m_UpperBound * (1 - macro->m_Solver->GetProbability(macro->m_BestChoices.front()));

            if (!prun)
                for (auto b : macro->m_BestChoices)
                    OpenBlock(macro, b);
        }
        delete macro->m_Solver;
        macro->m_Solver = nullptr;
        if (Queued[macro->m_Depth] == 0)
        {
            std::cout << "Layer " << macro->m_Depth << " cleared. Next Layer will be " << WorkingQueue.size() << std::endl;
            maxDepth = macro->m_Depth;
        }
    }

    for (auto i = maxDepth + 1; i < Situations.size(); ++i)
        Situations[i].clear();

    std::cout << "Merging..." << std::endl;
    
    GetUpperBound(root, maxDepth);

    std::cout << root->m_UpperBound << std::endl;
    system("pause");

    std::cout << "Clening..." << std::endl;

    for (auto layer : Situations)
        for (auto macro : layer)
            delete macro;

    return 0;
}
