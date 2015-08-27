#include "Solver.h"
#include "BinomialHelper.h"
#include <algorithm>
#include <assert.h>

static void Overlap(const BlockSet &setA, const BlockSet &setB, BlockSet &exceptA, BlockSet &exceptB, BlockSet &intersection);
static std::vector<int> Gauss(OrthogonalList<double> &matrix);
static void Merge(const std::vector<BigInteger> &from, std::vector<BigInteger> &to);
static void Add(std::vector<BigInteger> &from, const std::vector<BigInteger> &cases);
static unsigned __int64 Hash(const BlockSet &set);
template <class T>
static unsigned __int64 HashCol(const Node<T> *ptr);

Solver::Solver(int count) : m_Manager(count, BlockStatus::Unknown), m_Probability(count)
{
    m_BlockSets.emplace_back(count);
    auto &lst = m_BlockSets.back();
    for (auto i = 0; i < count; ++i)
        lst[i] = i;
    m_Matrix.ExtendWidth(2);
}

BlockStatus Solver::GetBlockStatus(Block block) const
{
    return m_Manager[block];
}

const BlockStatus *Solver::GetBlockStatuses() const
{
    return &*m_Manager.begin();
}

double Solver::GetProbability(Block block) const
{
    return m_Probability[block];
}

const double *Solver::GetProbabilities() const
{
    return &*m_Probability.begin();
}

const BigInteger &Solver::GetTotalStates() const
{
    return m_TotalStates;
}

void Solver::AddRestrain(Block blk, bool isMine)
{
    m_Manager[blk] = isMine ? BlockStatus::Mine : BlockStatus::Blank;
}

void Solver::AddRestrain(const BlockSet &set, int mines)
{
    auto theSet = BlockSet(set);
    sort(theSet.begin(), theSet.end());
    auto dMines = 0, dBlanks = 0;
    ReduceSet(theSet, dMines, dBlanks);

    m_Matrix.ExtendHeight(m_Matrix.GetHeight() + 1);

    auto col = 0;
    auto nr = &m_Matrix.GetRowHead(m_Matrix.GetHeight() - 1);
    for (auto it = m_BlockSets.begin(); it != m_BlockSets.end(); ++it , ++col)
    {
        BlockSet exceptA, exceptB, intersection;
        Overlap(*it, theSet, exceptA, exceptB, intersection);
        if (intersection.empty())
            continue;

        if (exceptA.empty())
        {
            it->swap(intersection);
            nr = &m_Matrix.Add(*nr, m_Matrix.GetColHead(col), 1);
        }
        else
        {
            it->swap(exceptA);
            if (!intersection.empty())
            {
                m_Matrix.InsertCol(col + 1);
                auto node = m_Matrix.GetColHead(col).Down;
                auto nodeX = &m_Matrix.GetColHead(col + 1);
                while (node != nullptr)
                {
                    nodeX = &m_Matrix.Add(*node, *nodeX, node->Value);
                    node = node->Down;
                }
                nr = &m_Matrix.Add(*nr, *nodeX, 1);

                ++col;
                if (++it == m_BlockSets.end())
                {
                    m_BlockSets.emplace_back();
                    it = m_BlockSets.end() - 1;
                }
                else
                {
                    m_BlockSets.insert(it, BlockSet());
                    it = m_BlockSets.begin() + col;
                }
                it->swap(intersection);
            }
        }
        theSet.swap(exceptB);

        if (theSet.empty())
            break;
    }


    if (!theSet.empty())
    {
        m_BlockSets.emplace_back();
        m_BlockSets.back().swap(theSet);

        m_Matrix.InsertCol(m_Matrix.GetWidth() - 1);
        m_Matrix.Add(*nr, m_Matrix.GetColHead(m_Matrix.GetWidth() - 1), 1);
    }

    m_Matrix.ExtendWidth(m_BlockSets.size() + 1);
    m_Matrix.Add(*nr, m_Matrix.GetColHead(m_BlockSets.size()), mines - dMines);
}

void Solver::Solve(bool withProb)
{
    m_Solutions.clear();
    m_DistCondCache.clear();
    m_DistCondQCache.clear();

    auto flag = true;
    while (flag)
    {
        while (ReduceRestrains()) {}
        flag = SimpleOverlap();
    }
    MergeSets();

    if (!withProb)
        return;

    if (m_BlockSets.size() == 0)
    {
        m_TotalStates = BigInteger(1);

        m_Solutions.emplace_back(std::vector<int>());
        ProcessSolutions();
        return;
    }

    auto augmentedMatrix = OrthogonalList<double>(m_Matrix);
    auto minors = Gauss(augmentedMatrix);

    if (!minors.empty() &&
        minors.back() == m_BlockSets.size())
        minors.pop_back();
    else
    {
        m_TotalStates = BigInteger(0);
        return;
    }

    EnumerateSolutions(minors, augmentedMatrix);
    if (m_Solutions.empty())
    {
        m_TotalStates = BigInteger(0);
        return;
    }

    ProcessSolutions();
}

const std::vector<BigInteger> &Solver::DistributionCond(const BlockSet &set, const BlockSet &setCond, int mines, int &min)
{
    int dMines, dBlanks;
    auto lst = BlockSet(set);
    ReduceSet(lst, dMines, dBlanks);

    int dMinesCond, dBlanksCond;
    auto lstCond = BlockSet(setCond);
    ReduceSet(lstCond, dMinesCond, dBlanksCond);

    if (lstCond.empty())
        if (dMinesCond != mines)
            throw;

    BlockSet sets1, sets2, sets3;
    GetIntersectionCounts(lst, lstCond, sets1, sets2, sets3);

    min = dMines;
    return DistCond(DistCondParameters(sets1, sets2, sets3, mines - dMinesCond, lst.size()));
}

const std::vector<BigInteger> &Solver::DistributionCondQ(const BlockSet &set, Block blk, int &min)
{
    int dMines, dBlanks;
    auto lst = BlockSet(set);
    ReduceSet(lst, dMines, dBlanks);

    int id;
    for (id = 0; id < m_BlockSets.size(); ++id)
        if (std::binary_search(m_BlockSets[id].begin(), m_BlockSets[id].end(), blk))
            break;

    BlockSet sets1;
    GetIntersectionCounts(lst, sets1);

    min = dMines;
    return DistCondQ(DistCondQParameters(sets1, id, lst.size()));
}

void Overlap(const BlockSet &setA, const BlockSet &setB, BlockSet &exceptA, BlockSet &exceptB, BlockSet &intersection)
{
    auto p = 0, q = 0;
    for (; p < setA.size() && q < setB.size();)
    {
        if (setA[p] < setB[q])
            exceptA.push_back(setA[p++]);
        else if (setA[p] > setB[q])
            exceptB.push_back(setB[q++]);
        else
        {
            intersection.push_back(setB[q]);
            p++;
            q++;
        }
    }
    while (p < setA.size())
        exceptA.push_back(setA[p++]);
    while (q < setB.size())
        exceptB.push_back(setB[q++]);
}

std::vector<int> Solver::OverlapBlockSet(const BlockSet &set)
{
    auto theSet = BlockSet(set);
    auto blkLst = std::vector<int>();
    blkLst.reserve(m_BlockSets.size());
    auto col = 0;
    for (auto it = m_BlockSets.begin(); it != m_BlockSets.end(); ++it , ++col)
    {
        BlockSet exceptA, exceptB, intersection;
        Overlap(*it, theSet, exceptA, exceptB, intersection);
        if (intersection.empty())
            continue;

        if (exceptA.empty())
        {
            it->swap(intersection);
            blkLst.push_back(col);
        }
        else
        {
            it->swap(exceptA);
            if (!intersection.empty())
            {
                auto node = m_Matrix.GetColHead(col).Down;
                while (node != nullptr)
                {
                    m_Matrix.Add(*node, m_Matrix.GetColHead(col), node->Value);
                    node = node->Down;
                }

                ++it , ++col;
                m_BlockSets.insert(it, BlockSet());
                it->swap(intersection);
                blkLst.push_back(col);
            }
        }
        if (exceptB.empty())
            return blkLst;

        theSet.swap(exceptB);
    }


    blkLst.push_back(col);
    m_BlockSets.emplace_back();
    m_BlockSets.back().swap(theSet);

    return blkLst;
}

void Solver::ReduceSet(BlockSet &set, int &mines, int &blanks) const
{
    mines = 0;
    blanks = 0;
    for (auto i = 0; i < set.size(); ++i)
        switch (m_Manager[set[i]])
        {
        case BlockStatus::Mine:
            mines++;
            set.erase(set.begin() + i--);
            break;
        case BlockStatus::Blank:
            blanks++;
            set.erase(set.begin() + i--);
            break;
        case BlockStatus::Unknown:
        default:
            break;
        }
}

void Solver::MergeSets()
{
    std::multimap<unsigned __int64, int> hash;
    for (auto i = 0; i < m_BlockSets.size(); ++i)
    {
        auto h = HashCol(m_Matrix.GetColHead(i).Down);
        auto itp = hash.equal_range(h);
        auto flag = false;
        for (auto it = itp.first; it != itp.second; ++it)
        {
            auto nc1 = m_Matrix.GetColHead(it->second).Down;
            auto nc2 = m_Matrix.GetColHead(i).Down;
            while (nc1 != nullptr && nc2 != nullptr)
            {
                if (nc1->Row != nc2->Row)
                    break;
                nc1 = nc1->Down;
                nc2 = nc2->Down;
            }
            if (nc1 == nullptr && nc2 == nullptr)
            {
                flag = true;
                m_BlockSets[it->second].reserve(m_BlockSets[it->second].size() + m_BlockSets[i].size());
                for (auto &blk : m_BlockSets[i])
                    m_BlockSets[it->second].push_back(blk);
                m_BlockSets.erase(m_BlockSets.begin() + i);
                m_Matrix.RemoveCol(i);
                --i;
                break;
            }
        }
        if (flag)
            hash.insert(std::make_pair(h, i));
    }
}

bool Solver::ReduceRestrains()
{
    auto flag = false;
    auto row = 0;
    auto n = m_Matrix.GetColHead(m_BlockSets.size()).Down;
    while (row < m_Matrix.GetHeight())
    {
        if (n == nullptr || n->Row > row || n->Value == 0)
        {
            auto nr = m_Matrix.GetRowHead(row).Right;
            while (nr->Col != n->Col)
            {
                auto col = nr->Col;
                for (auto &blk : m_BlockSets[col])
                    m_Manager[blk] = BlockStatus::Blank;
                nr = nr->Right;
                flag = true;
            }
            if (n != nullptr && n->Row == row)
                n = n->Down;

            m_Matrix.RemoveRow(row);
            continue;
        }
        auto count = 0;
        {
            auto nr = m_Matrix.GetRowHead(row).Right;
            while (nr != n)
            {
                count += m_BlockSets[nr->Col].size();
                nr = nr->Right;
            }
        }
        if (n->Value > count)
        {
            m_TotalStates = BigInteger(0);
            return true;
        }
        if (n->Value == count)
        {
            auto nr = m_Matrix.GetRowHead(row).Right;
            while (nr != n)
            {
                auto col = nr->Col;
                for (auto &blk : m_BlockSets[col])
                    m_Manager[blk] = BlockStatus::Mine;
                nr = nr->Right;
                flag = true;
            }
            assert(n != nullptr && n->Row == row);
            n = n->Down;

            m_Matrix.RemoveRow(row);
            continue;
        }
        assert(n != nullptr && n->Row == row);
        n = n->Down;
        row++;
    }

    for (auto col = 0; col < m_BlockSets.size(); ++col)
    {
        int dMines, dBlanks;
        ReduceSet(m_BlockSets[col], dMines, dBlanks);
        if (dMines != 0)
        {
            auto nc = m_Matrix.GetColHead(col).Down;
            auto ncc = &m_Matrix.GetColHead(m_BlockSets.size());
            while (nc != nullptr)
            {
                auto node = &m_Matrix.SeekDown(nc->Row, *ncc);
                if (node->Down == nullptr ||
                    node->Down->Row != nc->Row ||
                    (node->Down->Value -= dMines) < 0)
                {
                    m_TotalStates = BigInteger(0);
                    return true;
                }
                nc = nc->Down;
                ncc = node->Down;
            }
        }
        if (m_BlockSets[col].empty())
        {
            m_Matrix.RemoveCol(col);
            m_BlockSets.erase(m_BlockSets.begin() + col);
        }
    }
    return flag;
}

bool Solver::SimpleOverlap()
{
    auto flag = false;
    m_Pairs.clear();

    for (auto col = 0; col < m_BlockSets.size(); ++col)
    {
        std::vector<int> indexes;
        auto node = m_Matrix.GetColHead(col).Down;
        while (node != nullptr)
        {
            indexes.push_back(node->Row);
            node = node->Down;
        }

        for (auto i = 0; i < indexes.size() - 1; ++i)
            for (auto j = i + 1; j < indexes.size(); ++j)
                flag |= SimpleOverlap(indexes[i], indexes[j]);
    }

    return flag;
}

bool Solver::SimpleOverlap(int r1, int r2)
{
    if (!m_Pairs.insert(std::make_pair(r1, r2)).second)
        return false;

    std::vector<int> exceptA, exceptB, intersection;

    auto n1 = m_Matrix.GetRowHead(r1).Right, n2 = m_Matrix.GetRowHead(r2).Right;
    while (n1 != nullptr && n2 != nullptr)
    {
        if (n1->Col < n2->Col)
        {
            exceptA.push_back(n1->Col);
            n1 = n1->Right;
        }
        else if (n1->Col > n2->Col)
        {
            exceptB.push_back(n2->Col);
            n2 = n2->Right;
        }
        else
        {
            if (n1->Col == m_BlockSets.size())
                break;
            intersection.push_back(n1->Col);
            n1 = n1->Right;
            n2 = n2->Right;
        }
    }

    typedef std::pair<int, int> Iv;

    auto sum = [this](const std::vector<int> &lst)-> Iv
        {
            Iv iv(0, 0);
            for (const auto &index : lst)
                iv.second += m_BlockSets[index].size();
            return iv;
        };

    auto subs = [](Iv i1, Iv i2)-> Iv
        {
            return Iv(i1.first - i2.second, i1.second - i2.first);
        };
    auto ints = [](Iv i1, Iv i2)-> Iv
        {
            return Iv(max(i1.first, i2.first), min(i1.second, i2.second));
        };

    auto ivAC = Iv(n1->Value, n1->Value), ivBC = Iv(n2->Value, n2->Value);
    auto ivA0 = sum(exceptA), ivB0 = sum(exceptB), ivC0 = sum(intersection);
    auto ivA = ivA0, ivB = ivB0, ivC = ivC0;
    ivA = ints(ivA, subs(ivAC, ivC));
    ivB = ints(ivB, subs(ivBC, ivC));
    ivC = ints(ivC, ints(subs(ivAC, ivA), subs(ivBC, ivB)));
    ivA = ints(ivA, subs(ivAC, ivC));
    ivB = ints(ivB, subs(ivBC, ivC));
    ivC = ints(ivC, ints(subs(ivAC, ivA), subs(ivBC, ivB)));

    auto flag = false;
    auto proc = [&flag, this](const std::vector<int> &lst, const Iv &iv0,const Iv &iv)
        {
            if (lst.empty())
                return;
            if (iv0.second == iv.first)
            {
                for (const auto &id : lst)
                    for (const auto &blk : m_BlockSets[id])
                        m_Manager[blk] = BlockStatus::Mine;
                flag = true;
            }
            else if (iv0.first == iv.second)
            {
                for (const auto &id : lst)
                    for (const auto &blk : m_BlockSets[id])
                        m_Manager[blk] = BlockStatus::Blank;
                flag = true;
            }
        };

    proc(exceptA, ivA0, ivA);
    proc(exceptB, ivB0, ivB);
    proc(intersection, ivC0, ivC);

    return flag;
}

std::vector<int> Gauss(OrthogonalList<double> &matrix)
{
    auto n = matrix.GetWidth();
    auto m = matrix.GetHeight();
    auto minorCol = std::vector<int>();
    auto major = 0;
    for (auto col = 0; col < n; col++)
    {
        auto biasNode = matrix.SeekDown(major, col).Down;
        while (biasNode != nullptr && abs(biasNode->Value) < 1E-14)
        {
            auto tmp = biasNode->Down;
            matrix.Remove(*biasNode);
            biasNode = tmp;
        }
        if (biasNode == nullptr)
        {
            minorCol.push_back(col);
            continue;
        }

        auto vec = std::vector<double>(m);
        vec.resize(m, 0);
        auto theBiasInv = 1 / biasNode->Value;
        vec[biasNode->Row] = theBiasInv;
        auto node = matrix.GetColHead(col).Down;
        while (node != nullptr)
        {
            auto tmp = node->Down;
            if (node->Row != biasNode->Row)
            {
                vec[node->Row] = -node->Value * theBiasInv;
                matrix.Remove(*node);
            }
            else
                node->Value = 1;
            node = tmp;
        }

        auto bias = biasNode->Right;
        while (bias != nullptr)
        {
            auto biasVal = bias->Value;
            auto nc = &matrix.GetColHead(bias->Col);
            for (auto row = 0; row < m; row++)
            {
                double oldV;
                if (nc->Down == nullptr ||
                    nc->Down->Row > row)
                    oldV = 0;
                else
                    oldV = nc->Down->Value;

                auto val = (row != biasNode->Row ? oldV : 0) + vec[row] * biasVal;

                if (nc->Down == nullptr ||
                    nc->Down->Row > row)
                {
                    if (abs(val) < 1E-14)
                        continue;

                    auto nr = &matrix.SeekRight(row, bias->Col);

                    nc = &matrix.Add(*nr, *nc, val);
                }
                else if (abs(val) < 1E-14)
                    matrix.Remove(*nc->Down);
                else
                {
                    nc = nc->Down;
                    nc->Value = val;
                }
            }
            bias = bias->Right;
        }

        if (major != biasNode->Row)
        {
            auto majNode = &matrix.GetRowHead(major);
            auto biasRow = biasNode->Row;
            auto biaNode = &matrix.GetRowHead(biasRow);
            while (true)
            {
                auto majID = majNode->Right != nullptr ? majNode->Right->Col : n;
                auto biaID = biaNode->Right != nullptr ? biaNode->Right->Col : n;
                if (majID < biaID)
                {
                    biaNode = &matrix.Add(biasRow, majID, majNode->Right->Value);
                    matrix.Remove(*majNode->Right);
                }
                else if (majID > biaID)
                {
                    majNode = &matrix.Add(major, biaID, biaNode->Right->Value);
                    matrix.Remove(*biaNode->Right);
                }
                else if (majNode->Right != nullptr)
                {
                    majNode = majNode->Right;
                    biaNode = biaNode->Right;
                    std::swap(biaNode->Value, majNode->Value);
                }
                else
                    break;
            }
        }
        major++;
    }
    return minorCol;
}

void Solver::EnumerateSolutions(const std::vector<int> &minors, const OrthogonalList<double> &augmentedMatrix)
{
    auto n = m_BlockSets.size();

    if (minors.empty())
    {
        auto lst = std::vector<int>(n);
        auto nc = augmentedMatrix.GetColHead(n).Down;
        while (nc != nullptr)
        {
            lst[nc->Row] = static_cast<int>(round(nc->Value));
            nc = nc->Down;
        }
        m_Solutions.emplace_back(move(lst));
        return;
    }

    auto mR = n - minors.size();
    auto majors = std::vector<int>(mR);
    auto cnts = std::vector<int>(mR);
    auto sums = std::vector<double>(mR);
    {
        auto minorID = 0;
        auto mainRow = 0;
        auto nc = augmentedMatrix.GetColHead(n).Down;
        for (auto col = 0; col < n; col++)
            if (minorID < minors.size() &&
                col == minors[minorID])
                minorID++;
            else // major
            {
                majors[mainRow] = col;
                cnts[mainRow] = m_BlockSets[col].size();
                if (nc == nullptr || nc->Row > mainRow)
                    sums[mainRow] = 0;
                else
                {
                    sums[mainRow] = nc->Value;
                    nc = nc->Down;
                }
                mainRow++;
            }
    }
    auto aggr = [&sums,&minors,&augmentedMatrix](int minor, int val)
        {
            auto nc = augmentedMatrix.GetColHead(minors[minor]).Down;
            while (nc != nullptr)
            {
                sums[nc->Row] -= val * nc->Value;
                nc = nc->Down;
            }
        };
    auto stack = std::vector<int>();
    stack.reserve(minors.size());
    stack.push_back(0);
    while (true)
        if (stack.size() == minors.size())
            if (stack.back() <= m_BlockSets[minors.back()].size())
            {
                auto lst = std::vector<int>(n);
                auto flag = true;
                for (auto mainRow = 0; mainRow < mR; mainRow++)
                {
                    auto val = static_cast<int>(round(sums[mainRow]));
                    if (val < 0 ||
                        val > cnts[mainRow])
                    {
                        flag = false;
                        break;
                    }
                    lst[majors[mainRow]] = val;
                }
                if (flag)
                {
                    for (auto minorID = 0; minorID < minors.size(); minorID++)
                        lst[minors[minorID]] = stack[minorID];
                    m_Solutions.emplace_back(move(lst));
                }

                aggr(stack.size() - 1, 1);
                stack.back()++;
            }
            else
            {
                aggr(stack.size() - 1, -stack.back());
                stack.pop_back();
                if (stack.empty())
                    break;
                aggr(stack.size() - 1, 1);
                stack.back()++;
            }
        else if (stack.back() <= m_BlockSets[minors[stack.size() - 1]].size())
            stack.push_back(0); // aggr(stack.size() - 1, 0);
        else
        {
            aggr(stack.size() - 1, -stack.back());
            stack.pop_back();
            if (stack.empty())
                break;
            aggr(stack.size() - 1, 1);
            stack.back()++;
        }
}

void Solver::ProcessSolutions()
{
    auto exp = std::vector<BigInteger>(m_BlockSets.size(), 0);
    m_TotalStates = BigInteger(0);
    for (auto &so : m_Solutions)
    {
        so.States = BigInteger(1);
        for (auto i = 0; i < m_BlockSets.size(); ++i)
            so.States *= Binomial(m_BlockSets[i].size(), so.Dist[i]);
        m_TotalStates += so.States;
        for (auto i = 0; i < m_BlockSets.size(); ++i)
            exp[i] += so.States * so.Dist[i];
    }
    for (auto &so : m_Solutions)
        so.Ratio = so.States / m_TotalStates;
    for (auto i = 0; i < m_Manager.size(); ++i)
        if (m_Manager[i] == BlockStatus::Mine)
            m_Probability[i] = 1;
        else if (m_Manager[i] == BlockStatus::Blank)
            m_Probability[i] = 0;

    for (auto i = 0; i < m_BlockSets.size(); ++i)
    {
        auto prod = m_TotalStates * m_BlockSets[i].size();
        if (prod < exp[i])
            throw;
        if (exp[i] == 0)
            for (auto &blk : m_BlockSets[i])
                m_Manager[blk] = BlockStatus::Blank;
        else if (exp[i] == prod)
            for (auto &blk : m_BlockSets[i])
                m_Manager[blk] = BlockStatus::Mine;

        auto p = exp[i] / prod;
        for (auto &blk : m_BlockSets[i])
            m_Probability[blk] = p;
    }
}

void Merge(const std::vector<BigInteger> &from, std::vector<BigInteger> &to)
{
    for (auto i = 0; i < from.size(); ++i)
        to[i] += from[i];
}

void Add(std::vector<BigInteger> &from, const std::vector<BigInteger> &cases)
{
    auto dicN = std::vector<BigInteger>(from.size() + cases.size() - 1, BigInteger(0));
    for (auto i = 0; i < from.size(); i++)
        for (auto j = 0; j < cases.size(); j++)
            dicN[i + j] += from[i] * cases[j];
    dicN.swap(from);
}

void Solver::GetIntersectionCounts(const BlockSet &set1, std::vector<int> &sets1) const
{
    auto lst1 = BlockSet(set1);
    sets1.clear();
    sets1.reserve(m_BlockSets.size());
    for (auto &set : m_BlockSets)
    {
        auto inter = 0;

        auto p = 0, q = 0;
        for (; p < set.size() && q < lst1.size();)
        {
            if (set[p] < lst1[q])
                ++p;
            else if (set[p] > lst1[q])
                ++q;
            else
            {
                ++inter;
                ++p;
                lst1.erase(lst1.begin() + q);
            }
        }

        sets1.push_back(inter);
    }
}

void Solver::GetIntersectionCounts(const BlockSet &set1, const BlockSet &set2, std::vector<int> &sets1, std::vector<int> &sets2, std::vector<int> &sets3) const
{
    auto lst1 = BlockSet(set1), lst2 = BlockSet(set2);
    sets1.clear();
    sets1.reserve(m_BlockSets.size());
    sets2.clear();
    sets2.reserve(m_BlockSets.size());
    sets3.clear();
    sets3.reserve(m_BlockSets.size());
    for (auto &set : m_BlockSets)
    {
        BlockSet exceptA, exceptB1T, exceptB2T, exceptB1, exceptB2, exceptC, resume1, resume2;
        Overlap(set, lst1, exceptA, resume1, exceptB1T);
        exceptA.clear();
        Overlap(set, lst2, exceptA, resume2, exceptB2T);
        Overlap(exceptB1T, exceptB2T, exceptB1, exceptB2, exceptC);
        lst1.swap(resume1);
        lst2.swap(resume2);
        sets1.push_back(exceptB1.size());
        sets2.push_back(exceptB2.size());
        sets3.push_back(exceptC.size());
    }
}

const std::vector<BigInteger> &Solver::DistCond(const DistCondParameters &par)
{
    auto itp = m_DistCondCache.equal_range(par.m_Hash);
    for (auto it = itp.first; it != itp.second; ++it)
    {
        if (it->second.first == par)
            return it->second.second;
    }
    std::vector<BigInteger> dic(par.Length + 1);
    for (auto &solution : m_Solutions)
    {
        auto max = std::vector<int>(m_BlockSets.size());
        for (auto i = 0; i < m_BlockSets.size(); ++i)
            max[i] = min(solution.Dist[i], par.Sets2[i] + par.Sets3[i]);

        auto stack = std::vector<int>();
        stack.reserve(m_BlockSets.size());
        if (m_BlockSets.size() > 1)
            stack.push_back(0);
        while (true)
            if (stack.size() == m_BlockSets.size() - 1)
            {
                auto mns = par.MinesCond;
                for (auto v : stack)
                    mns -= v;
                if (mns >= 0 &&
                    mns <= max[m_BlockSets.size() - 1])
                {
                    stack.push_back(mns);

                    auto dicT = std::vector<BigInteger>(1, BigInteger(1));
                    for (auto i = 0; i < m_BlockSets.size(); i++)
                    {
                        auto n = m_BlockSets[i].size();
                        auto a = par.Sets1[i], b = par.Sets2[i], c = par.Sets3[i];
                        auto m = solution.Dist[i], p = stack[i];

                        {
                            auto cases = std::vector<BigInteger>();
                            cases.reserve(min(p, c) + 1);
                            for (auto j = 0; j <= p && j <= c; j++)
                            {
                                cases.emplace_back(Binomial(c, j));
                                cases.back() *= Binomial(b, p - j);
                            }

                            Add(dicT, cases);
                        }
                        {
                            auto cases = std::vector<BigInteger>();
                            cases.reserve(min(m - p, a) + 1);
                            for (auto j = 0; j <= m - p && j <= a; j++)
                            {
                                cases.emplace_back(Binomial(a, j));
                                cases.back() *= Binomial(n - a - b - c, m - p - j);
                            }

                            Add(dicT, cases);
                        }
                    }
                    Merge(dicT, dic);

                    stack.pop_back();
                }
                if (stack.empty())
                    break;
                if (mns <= 0)
                {
                    stack.pop_back();
                    if (stack.empty())
                        break;
                }
                stack.back()++;
            }
            else if (stack.back() <= max[stack.size() - 1])
                stack.push_back(0);
            else
            {
                stack.pop_back();
                if (stack.empty())
                    break;
                stack.back()++;
            }
    }
    auto it = m_DistCondCache.insert(std::move(std::make_pair(par.m_Hash, std::make_pair(par, dic))));
    return it->second.second;
}

const std::vector<BigInteger> &Solver::DistCondQ(const DistCondQParameters &par)
{
    auto itp = m_DistCondQCache.equal_range(par.m_Hash);
    for (auto it = itp.first; it != itp.second; ++it)
    {
        if (it->second.first == par)
            return it->second.second;
    }
    std::vector<BigInteger> dic(par.Length + 1);
    for (auto &solution : m_Solutions)
    {
        auto dicT = std::vector<BigInteger>(1, BigInteger(1));
        for (auto i = 0; i < m_BlockSets.size(); i++)
        {
            auto n = m_BlockSets[i].size();
            auto a = par.Sets1[i], b = i == par.Set2ID ? 1 : 0;
            auto m = solution.Dist[i];

            auto cases = std::vector<BigInteger>();
            cases.reserve(min(m, a) + 1);
            for (auto j = 0; j <= m && j <= a; j++)
            {
                cases.emplace_back(Binomial(a, j));
                cases.back() *= Binomial(n - a - b, m - j);
            }

            Add(dicT, cases);
        }
        Merge(dicT, dic);
    }
    auto it = m_DistCondQCache.insert(std::move(std::make_pair(par.m_Hash, std::make_pair(par, dic))));
    return it->second.second;
}

DistCondParameters::DistCondParameters(const std::vector<Block> &sets1, const std::vector<Block> &sets2, const std::vector<Block> &sets3, int minesCond, int length)
    : Sets1(sets1),
      Sets2(sets2),
      Sets3(sets3),
      MinesCond(minesCond),
      Length(length)
{
    m_Hash = (Hash(Sets1) << 44) ^ (Hash(Sets2) << 24) ^ (Hash(Sets3) << 4) ^ MinesCond;
}

bool operator==(const DistCondParameters &lhs, const DistCondParameters &rhs)
{
    if (lhs.m_Hash != rhs.m_Hash)
        return false;
    if (lhs.MinesCond != rhs.MinesCond)
        return false;
    if (lhs.Sets1 != rhs.Sets1)
        return false;
    if (lhs.Sets2 != rhs.Sets2)
        return false;
    if (lhs.Sets3 != rhs.Sets3)
        return false;
    return true;
}

bool operator!=(const DistCondParameters &lhs, const DistCondParameters &rhs)
{
    return !(lhs == rhs);
}

bool operator<(const DistCondParameters &lhs, const DistCondParameters &rhs)
{
    if (lhs.m_Hash < rhs.m_Hash)
        return true;
    if (lhs.m_Hash > rhs.m_Hash)
        return false;
    if (lhs.MinesCond < rhs.MinesCond)
        return true;
    if (lhs.MinesCond > rhs.MinesCond)
        return false;
    if (lhs.Sets1 < rhs.Sets1)
        return true;
    if (lhs.Sets1 > rhs.Sets1)
        return false;
    if (lhs.Sets2 < rhs.Sets2)
        return true;
    if (lhs.Sets2 > rhs.Sets2)
        return false;
    if (lhs.Sets3 < rhs.Sets3)
        return true;
    if (lhs.Sets3 > rhs.Sets3)
        return false;
    return false;
}

DistCondQParameters::DistCondQParameters(const std::vector<int> &sets1, Block set2ID, int length)
    : Sets1(sets1),
      Set2ID(set2ID),
      Length(length)
{
    m_Hash = (Hash(Sets1) << 8) ^ set2ID;
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

unsigned __int64 Hash(const BlockSet &set)
{
    unsigned __int64 hash = 5381;
    auto it = set.begin();
    while (it != set.end())
        hash = (hash << 5) + hash + *it++ + 30;
    return hash;
}

template <class T>
unsigned __int64 HashCol(const Node<T> *ptr)
{
    unsigned __int64 hash = 5381;
    while (ptr != nullptr)
    {
        hash = (hash << 5) + hash + ptr->Row;
        ptr = ptr->Down;
    }
    return hash;
}
