#include "Solver.h"
#include <algorithm>
#include "BinomialHelper.h"

static size_t Hash(const BlockSet &set);

Solver::Solver(size_t count) : BasicSolver(count) {}

Solver::Solver(size_t count, int mines) : BasicSolver(count, mines) {}

Solver::Solver(const Solver &other) : BasicSolver(other) {}

Solver::~Solver()
{
    ClearDistCondQCache();
}

bool Solver::Solve(SolvingState maxDepth, bool shortcut)
{
    if (BasicSolver::Solve(maxDepth, shortcut))
    {
        ClearDistCondQCache();
        return true;
    }
    return false;
}

const DistCondQParameters &Solver::GetDistInfo(const BlockSet &set, Block blk, int &min)
{
    return UCondQ(PackParameters(set, blk, min));
}

double Solver::ZeroCondQ(const BlockSet &set, Block blk)
{
    int min;
    return ZCondQ(PackParameters(set, blk, min)).m_Result.front();
}

double Solver::ZerosCondQ(const BlockSet &set, Block blk)
{
    int min;
    return UCondQ(PackParameters(set, blk, min)).m_Probability;
}

double Solver::ZerosECondQ(const BlockSet &set, Block blk)
{
    int min;
    return UCondQ(PackParameters(set, blk, min)).m_Expectation;
}

double Solver::UpperBoundCondQ(const BlockSet &set, Block blk)
{
    int min;
    return UCondQ(PackParameters(set, blk, min)).m_UpperBound;
}

const std::vector<double> &Solver::DistributionCondQ(const BlockSet &set, Block blk, int &min)
{
    return DistCondQ(PackParameters(set, blk, min)).m_Result;
}

double Solver::QuantityCondQ(const BlockSet &set, Block blk)
{
    DistCondQParameters par(m_SetIDs[blk], 0);
    int dMines;
    GetIntersectionCounts(set, par.Sets1, dMines);
    par.Hash();
    for (auto b : par.Sets1)
        par.Length += b;
    auto &di = DistCondQ(std::move(par));

    double q = 0;
    for (auto j = 0; j < di.m_Result.size(); ++j)
        if (di.m_Result[j] != 0)
        {
            auto p = di.m_Result[j] / di.m_TotalStates;
            q += -p * log2(p);
        }

    return q;
}

void Solver::Merge(const std::vector<double> &from, std::vector<double> &to)
{
    ASSERT(from.size() <= to.size());
    for (auto i = 0; i < from.size(); ++i)
        to[i] += from[i];
}

void Solver::Add(std::vector<double> &from, const std::vector<double> &cases)
{
    auto &dicN = m_Add_Temp;
    dicN.clear() , dicN.resize(from.size() + cases.size() - 1, 0);
    for (auto i = 0; i < from.size(); ++i)
        for (auto j = 0; j < cases.size(); ++j)
            dicN[i + j] += from[i] * cases[j];
    dicN.swap(from);
}

DistCondQParameters Solver::PackParameters(const BlockSet &set, Block blk, int &min) const
{
    DistCondQParameters par(m_SetIDs[blk], 0);
    int dMines;
    GetIntersectionCounts(set, par.Sets1, dMines);
    par.Hash();
    for (auto b : par.Sets1)
        par.Length += b;
    min = dMines;
    return par;
}

void Solver::GetHalves(DistCondQParameters &par) const
{
    par.m_Halves.clear();
    for (auto i = 0; i < par.Sets1.size(); ++i)
        if (par.Sets1[i] != 0 && par.Sets1[i] != m_BlockSets[i].size() &&
            !(i == par.Set2ID && par.Sets1[i] == m_BlockSets[i].size() - 1))
            par.m_Halves.push_back(i);
}

void Solver::EnumerateSolutions(DistCondQParameters &par) const
{
    par.m_States.clear();
    par.m_States.resize(par.Length + 1, 0);
    par.m_Solutions.clear();
    par.m_Solutions.resize(par.Length + 1);

    std::vector<int> stack, lb, ub;
    for (auto &solution : m_Solutions)
    {
        if (par.Set2ID > 0 && m_BlockSets[par.Set2ID].size() == solution.Dist[par.Set2ID])
            continue;

        lb.clear() , ub.clear();
        for (auto id : par.m_Halves)
        {
            lb.push_back(MAX(static_cast<int>(solution.Dist[id]) - static_cast<int>(m_BlockSets[id].size()) + (id == par.Set2ID ? 1 : 0) + par.Sets1[id], 0));
            ub.push_back(MIN(solution.Dist[id], par.Sets1[id]));
        }

        stack.clear();
        stack.reserve(par.m_Halves.size());
        if (!lb.empty())
            stack.push_back(lb.front());
        while (true)
            if (stack.size() == par.m_Halves.size())
                if (par.m_Halves.empty() || stack.back() <= ub[stack.size() - 1])
                {
                    auto val = 0;
                    double st = 1;
                    std::vector<int> dist(par.Sets1.size() + par.m_Halves.size());
                    for (auto i = 0, p = 0; i < par.Sets1.size(); ++i)
                    {
                        if (p < par.m_Halves.size() && i == par.m_Halves[p])
                        {
                            val += dist[i] = stack[p];
                            dist[par.Sets1.size() + p] = solution.Dist[i] - stack[p];
                            st *= Binomial(par.Sets1[i], dist[i]);
                            st *= Binomial(m_BlockSets[i].size() - (i == par.Set2ID ? 1 : 0) - par.Sets1[i], dist[par.Sets1.size() + p]);
                            ++p;
                        }
                        else
                        {
                            dist[i] = solution.Dist[i];
                            st *= Binomial(m_BlockSets[i].size() - (i == par.Set2ID ? 1 : 0), dist[i]);
                            if (par.Sets1[i] != 0)
                                val += dist[i];
                        }
                    }
                    if (st > 0)
                    {
                        par.m_States[val] += st;
                        par.m_Solutions[val].emplace_back();
                        par.m_Solutions[val].back().Dist.swap(dist);
                        par.m_Solutions[val].back().States = st;
                    }

                    if (par.m_Halves.empty())
                        break;
                    ++stack.back();
                }
                else
                {
                    stack.pop_back();
                    if (stack.empty())
                        break;
                    ++stack.back();
                }
            else if (stack.back() <= ub[stack.size() - 1])
                stack.push_back(lb[stack.size()]);
            else
            {
                stack.pop_back();
                if (stack.empty())
                    break;
                ++stack.back();
            }
    }

    par.m_TotalStates = 0;
    for (auto val : par.m_States)
        par.m_TotalStates += val;

    par.m_Result.clear();
    par.m_Result.reserve(par.m_States.size());
    for (auto val : par.m_States)
        par.m_Result.push_back(val / par.m_TotalStates);
}

DistCondQParameters *Solver::TryGetCache(DistCondQParameters &&par, std::function<bool(const DistCondQParameters &)> pre)
{
    DistCondQParameters *ptr = nullptr;
    auto itp = m_DistCondQCache.equal_range(par.m_Hash);
    for (auto it = itp.first; it != itp.second; ++it)
    {
        if (*it->second != par)
            continue;
        if (pre(*it->second))
            return it->second;
        ptr = it->second;
        break;
    }
    if (ptr == nullptr)
    {
        ptr = new DistCondQParameters(std::move(par));
        m_DistCondQCache.insert(std::make_pair(ptr->m_Hash, ptr));
    }
    return ptr;
}

const DistCondQParameters &Solver::ZCondQ(DistCondQParameters &&par)
{
    auto pre = [](const DistCondQParameters &p)
        {
            return !p.m_Result.empty();
        };
    auto ptr = TryGetCache(std::move(par), pre);
    if (pre(*ptr))
        return *ptr;

    double val = 0;
    for (auto &solution : m_Solutions)
    {
        double valT = 1;
        for (auto i = 0; i < m_BlockSets.size(); ++i)
        {
            auto n = m_BlockSets[i].size();
            auto a = ptr->Sets1[i], b = i == ptr->Set2ID ? 1 : 0;
            auto m = solution.Dist[i];

            valT *= Binomial(n - a - b, m);
        }
        val += valT;
    }
    ptr->m_Result.push_back(val);
    return *ptr;
}

const DistCondQParameters &Solver::DistCondQ(DistCondQParameters &&par)
{
    auto pre = [](const DistCondQParameters &p)
        {
            return p.m_Result.size() == p.Length + 1;
        };
    auto ptr = TryGetCache(std::move(par), pre);
    if (pre(*ptr))
        return *ptr;

    GetHalves(*ptr);
    EnumerateSolutions(*ptr);
    return *ptr;
}

const DistCondQParameters &Solver::UCondQ(DistCondQParameters &&par)
{
    auto pre = [](const DistCondQParameters &p)
        {
            return !std::isnan(p.m_UpperBound);
        };
    auto ptr = TryGetCache(std::move(par), pre);
    if (pre(*ptr))
        return *ptr;

    GetHalves(*ptr);
    EnumerateSolutions(*ptr);

    ptr->m_Probability = 0;
    ptr->m_Expectation = 0;
    ptr->m_UpperBound = 0;
    std::vector<int> zero;
    std::vector<double> upper;
    for (auto i = 0; i <= ptr->Length; ++i)
    {
        if (ptr->m_Solutions[i].empty())
            continue;

        zero.clear();
        zero.resize(ptr->Sets1.size() + ptr->m_Halves.size(), 1);
        upper.clear();
        upper.resize(ptr->Sets1.size() + ptr->m_Halves.size(), 0);
        for (auto j = 0; j < ptr->m_Solutions[i].size(); ++j)
            for (auto k = 0; k < ptr->Sets1.size() + ptr->m_Halves.size(); ++k)
                if (ptr->m_Solutions[i][j].Dist[k] != 0)
                {
                    zero[k] = 0;
                    upper[k] += ptr->m_Solutions[i][j].Dist[k] * ptr->m_Solutions[i][j].States;
                }

        auto totalBlanks = 0;
        auto p = 0;
        for (auto j = 0; j < ptr->Sets1.size(); ++j)
        {
            while (p < ptr->m_Halves.size() && j > ptr->m_Halves[p])
                ++p;

            int size;
            if (p < ptr->m_Halves.size() && j == ptr->m_Halves[p])
            {
                size = ptr->Sets1[j];
                ++p;
            }
            else
            {
                size = m_BlockSets[j].size();
            }

            if (zero[j] == 1)
                totalBlanks += size;
            else
                upper[j] /= size;
        }
        for (auto j = ptr->Sets1.size(); j < ptr->Sets1.size() + ptr->m_Halves.size(); ++j)
        {
            auto size = m_BlockSets[j - ptr->Sets1.size()].size() - ptr->Sets1[j - ptr->Sets1.size()];
            if (j == ptr->Sets1.size() + ptr->Set2ID)
                --size;

            if (zero[j] == 1)
                totalBlanks += size;
            else
                upper[j] /= size;
        }

        if (totalBlanks != 0)
        {
            ptr->m_Probability += ptr->m_Result[i];
            ptr->m_Expectation += ptr->m_Result[i] * totalBlanks;
            continue;
        }

        auto bound = upper.front();
        for (auto u : upper)
            if (u < bound)
                bound = u;

        ptr->m_UpperBound += bound;
    }

    ptr->m_UpperBound = 1 - ptr->m_UpperBound / ptr->m_TotalStates;
    return *ptr;
}

void Solver::ClearDistCondQCache()
{
    for (auto &cache : m_DistCondQCache)
    {
        if (cache.second == nullptr)
            continue;
        delete cache.second;
        cache.second = nullptr;
    }
    m_DistCondQCache.clear();
}

DistCondQParameters::DistCondQParameters(DistCondQParameters &&other) noexcept : Sets1(std::move(other.Sets1)), Set2ID(other.Set2ID), Length(other.Length), m_Hash(other.m_Hash), m_Halves(std::move(other.m_Halves)), m_Result(std::move(other.m_Result)), m_Probability(other.m_Probability), m_Expectation(other.m_Expectation), m_UpperBound(other.m_UpperBound), m_TotalStates(other.m_TotalStates) {}

DistCondQParameters::DistCondQParameters(Block set2ID, int length) : Set2ID(set2ID), Length(length), m_Hash(Hash()), m_Probability(NAN), m_Expectation(NAN), m_UpperBound(NAN), m_TotalStates(NAN) {}

size_t DistCondQParameters::Hash()
{
    return m_Hash = ::Hash(Sets1) << 5 ^ Set2ID;
}

bool operator==(const DistCondQParameters &lhs, const DistCondQParameters &rhs)
{
    if (lhs.m_Hash != rhs.m_Hash)
        return false;
    if (lhs.Set2ID != rhs.Set2ID)
        return false;
    //if (lhs.Sets1 != rhs.Sets1)
    //    return false;
    return true;
}

bool operator!=(const DistCondQParameters &lhs, const DistCondQParameters &rhs)
{
    return !(lhs == rhs);
}

bool operator<(const DistCondQParameters &lhs, const DistCondQParameters &rhs)
{
    if (lhs.m_Hash < rhs.m_Hash)
        return true;
    if (lhs.m_Hash > rhs.m_Hash)
        return false;
    if (lhs.Set2ID < rhs.Set2ID)
        return true;
    if (lhs.Set2ID > rhs.Set2ID)
        return false;
    return false;
}

size_t Hash(const BlockSet &set)
{
    size_t hash = 5381;
    for (auto v : set)
        hash = (hash << 5) + hash + v + 30;
    return hash;
}
