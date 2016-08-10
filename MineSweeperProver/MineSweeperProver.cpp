// MineSweeperProver.cpp : �������̨Ӧ�ó������ڵ㡣
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
#include "../MineSweeperSolver/BasicDrainer.h"
#include <thread>
#include <mutex>
#include <queue>

size_t MemoryLimit;
double MinInfluence;

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

struct Macro;

struct Choice
{
    explicit Choice(int blk) : m_UpperBound(NAN), Block(blk) { }

    double m_UpperBound;
    int Block;
    std::unordered_map<Macro *, double> Next;
};

struct Macro
{
    Macro() : m_Depth(0), m_Solver(nullptr), m_UpperBound(NAN), m_Influence(0) { }

    Macro(const Macro &other) : m_Depth(other.m_Depth + 1), m_Degrees(other.m_Degrees), m_Solver(nullptr), m_UpperBound(NAN), m_Influence(other.m_Influence)
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
    double m_Influence;
};

bool operator<(const Macro &lhs, const Macro &rhs)
{
    return lhs.m_Influence < rhs.m_Influence;
}

bool operator>(const Macro &lhs, const Macro &rhs)
{
    return rhs.m_Influence < lhs.m_Influence;
}

bool operator<=(const Macro &lhs, const Macro &rhs)
{
    return !(lhs.m_Influence > rhs.m_Influence);
}

bool operator>=(const Macro &lhs, const Macro &rhs)
{
    return !(lhs.m_Influence < rhs.m_Influence);
}

struct Hash
{
    size_t operator()(Macro * const &macro) const
    {
        size_t h = 5381;
        for (auto v : macro->m_Degrees)
            h = (h << 5) + h + v;
        return h;
    }
};

bool operator==(const Macro &lhs, const Macro &rhs)
{
    return lhs.m_Degrees == rhs.m_Degrees;
}

bool operator!=(const Macro &lhs, const Macro &rhs)
{
    return !(lhs == rhs);
}

std::mutex WorkingQueueMutex;
std::priority_queue<Macro *> WorkingQueue;
std::mutex SituationsMutex;
std::vector<std::unordered_set<Macro *, Hash>> Situations;
std::vector<size_t> Queued;

void OpenBlock(Macro *macro, Block blk, bool forceNoMine = false)
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

        double prob;
        if (forceNoMine)
            prob = info.m_Result[i];
        else
            prob = info.m_Result[i] * (1 - macro->m_Solver->GetProbability(blk));

        auto child = new Macro(*macro);
        child->m_Degrees[blk] = min + i;

        std::unordered_set<Macro*, Hash>::_Pairib res;
        {
            std::unique_lock<std::mutex> lock(SituationsMutex);

            if (child->m_Depth >= Situations.size())
            {
                Situations.emplace_back();
                Queued.push_back(0);
            }

            res = Situations[child->m_Depth].insert(child);
            if (res.second)
                ++Queued[child->m_Depth];
        }
        if (res.second)
        {
            if (child->m_Solver->GetBlockStatus(blk) == BlockStatus::Blank)
                --child->m_Solver->CanOpenForSure;
            child->m_Solver->AddRestrain(blk, false);
            child->m_Solver->AddRestrain(m_BlocksR[blk], min + i);
        }
        else
        {
            delete child;
            child = *res.first;
        }
        macro->m_Choices.back().Next.insert(std::make_pair(child, prob));
        child->m_Influence = res.second ? macro->m_Influence * prob : max(child->m_Influence, macro->m_Influence * prob);

        {
            std::unique_lock<std::mutex> lock(WorkingQueueMutex);

            WorkingQueue.push(child);
        }
    }
}

void GetUpperBound(Macro *macro)
{
#ifndef _DEBUG
    if (macro->m_Solver != nullptr)
    {
        delete macro->m_Solver;
        macro->m_Solver = nullptr;
    }
#endif

    if (macro->m_Choices.empty())
        return;

    double upper = 0;
    for (auto &choice : macro->m_Choices)
    {
        double b = 0;
        for (auto &kvp : choice.Next)
        {
            GetUpperBound(kvp.first);
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

class SimpleDrainer : public BasicDrainer
{
public:
    explicit SimpleDrainer(const Macro &macro) : BasicDrainer()
    {
        this->m_BlocksR = ::m_BlocksR;
        GenerateMicros(macro.m_Solver->GetBlockSets(), macro.m_Solver->GetTotalStates(), macro.m_Solver->GetSolutions());
        GenerateRoot(new Solver(*macro.m_Solver), Width * Height - Mines - macro.m_Depth);
        Drain();
    }

protected:
    void HeuristicPruning(MacroSituation *macro, BlockSet &bests) override { }
};

std::mutex PrunMutex;
auto prun = false;

void Process()
{
    BlockSet preferred;
    while (true)
    {
        if (!prun && !IsMemoryEnough())
        {
            std::unique_lock<std::mutex> lock(PrunMutex);

            if (!prun)
            {
                std::cout << "(P)";
                prun = true;
            }
        }
        Macro *macro = nullptr;
        {
            std::unique_lock<std::mutex> lock(WorkingQueueMutex);

            if (!WorkingQueue.empty())
            {
                macro = WorkingQueue.top();
                WorkingQueue.pop();
            }
        }
        if (macro == nullptr)
        {
            std::this_thread::sleep_for(std::chrono::seconds{1});
            {
                std::unique_lock<std::mutex> lock(WorkingQueueMutex);

                if (WorkingQueue.empty())
                    break;
            }
        }
        {
            std::unique_lock<std::mutex> lock(WorkingQueueMutex);

            --Queued[macro->m_Depth];
        }
        macro->m_Solver->Solve(SolvingState::Reduce | SolvingState::Overlap | SolvingState::Probability, false);
        if (macro->m_Solver->GetTotalStates() == 1.0)
        {
            macro->m_UpperBound = 1;
        }
        else if (macro->m_Solver->CanOpenForSure > 0)
        {
            macro->m_UpperBound = 1;
            if (!prun && macro->m_Depth <= 2)
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

            if (!prun && macro->m_Influence > MinInfluence && macro->m_Depth <= 2)
#ifndef SEMI
                for (auto i = 0; i < Width * Height; ++i)
                    if (macro->m_Degrees[i] == CLOSED && macro->m_Solver->GetBlockStatus(i) == BlockStatus::Unknown)
                        OpenBlock(macro, i, macro->m_Depth == 0);
#else
                for (auto b : macro->m_BestChoices)
                    OpenBlock(macro, b);
#endif
        }
#ifndef _DEBUG
        delete macro->m_Solver;
        macro->m_Solver = nullptr;
#endif
    }
}

int main()
{
    int thrs;
#ifndef NDEBUG
    thrs = 1;
    Width = 8, Height = 8, Mines = 10;
    MemoryLimit = 2048 << 20;
    MinInfluence = 0;
#else
    std::cout << "Threads: ";
    std::cin >> thrs;

    std::cout << "Width Height Mines: ";
    std::cin >> Width >> Height >> Mines;

    std::cout << "Max Memory Usage (MB): ";
    std::cin >> MemoryLimit;
    MemoryLimit <<= 20;

    std::cout << "MinInfluence: ";
    std::cin >> MinInfluence;
#endif

    CacheBinomials(Width * Height, Mines);

    Situations.resize(10);
    Queued.resize(10);

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

    auto root = new Macro;
    root->m_Degrees.resize(Width * Height, CLOSED);
    root->m_Solver = new Solver(Width * Height, Mines);
    root->m_Solver->Solve(SolvingState::Reduce | SolvingState::Overlap | SolvingState::Probability, false);

    Situations[0].insert(root);
    root->m_Influence = 1;
    //OpenBlock(root, 0, true);
    WorkingQueue.push(root);
    ++Queued[0];

    //std::cout << "Draining..." << std::endl;
    //SimpleDrainer dr(*root);
    //std::cout << dr.GetBestProb() * (Width * Height) / (Width * Height - Mines) << std::endl;
    //system("pause");

    std::cout << "Forking..." << std::endl;

    std::vector<std::thread> thr;
    thr.reserve(thrs);
    for (auto i = 0; i < thrs; ++i)
        thr.emplace_back(&Process);
    for (auto &th : thr)
        th.join();

    std::cout << "Merging..." << std::endl;

    GetUpperBound(root);

    std::cout << root->m_UpperBound << std::endl;
    system("pause");

    //std::cout << "Clening..." << std::endl;

    //for (auto layer : Situations)
    //    for (auto macro : layer)
    //        delete macro;

    return 0;
}