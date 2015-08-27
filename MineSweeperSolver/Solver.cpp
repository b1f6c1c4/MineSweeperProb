#include "Solver.h"
#include "BinomialHelper.h"
#include <assert.h>

static std::vector<int> Gauss(OrthogonalList<int> &matrix);
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
    m_SetIDs.resize(count, 0);
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
    auto dMines = 0, dBlanks = 0;
    std::vector<int> bin;
    GetIntersectionCounts(set, bin, dMines, dBlanks);

    m_Matrix.ExtendHeight(m_Matrix.GetHeight() + 1);

    auto nr = &m_Matrix.GetRowHead(m_Matrix.GetHeight() - 1);
    for (auto col = 0; col < bin.size(); ++col)
    {
        if (bin[col] == 0)
            continue;

        if (bin[col] == m_BlockSets[col].size())
        {
            nr = &m_Matrix.Add(*nr, m_Matrix.GetColHead(col), 1);
            continue;
        }
        
        m_Matrix.InsertCol(m_Matrix.GetWidth() - 1);
        auto node = m_Matrix.GetColHead(col).Down;
        auto nodeX = &m_Matrix.GetColHead(m_Matrix.GetWidth() - 2);
        while (node != nullptr)
        {
            nodeX = &m_Matrix.Add(*node, *nodeX, node->Value);
            node = node->Down;
        }
        m_Matrix.Add(*nr, *nodeX, 1);

        m_BlockSets.emplace_back();
        auto &it = m_BlockSets.back();
        it.reserve(bin[col]);

        auto p = 0, q = 0;
        for (; p < set.size() && q < m_BlockSets[col].size();)
        {
            if (set[p] < m_BlockSets[col][q])
                ++p;
            else if (set[p] > m_BlockSets[col][q])
                ++q;
            else
            {
                it.push_back(set[p]);
                m_BlockSets[col].erase(m_BlockSets[col].begin() + q);
                m_SetIDs[set[p++]] = m_BlockSets.size() - 1;
            }
        }
    }

    m_Matrix.Add(*nr, m_Matrix.GetColHead(m_BlockSets.size()), mines - dMines);
}

void Solver::Solve(bool withOverlap, bool withProb)
{
    m_Solutions.clear();
    m_DistCondQCache.clear();

    auto flag = true;
    while (flag)
    {
        while (ReduceRestrains()) {}
        if (withOverlap)
            flag = SimpleOverlapAll();
        else
            flag = false;
    }
    MergeSets();

    if (!withProb)
        return;

    if (m_BlockSets.empty())
    {
        m_TotalStates = BigInteger(1);

        m_Solutions.emplace_back(std::vector<int>());
        ProcessSolutions();
        return;
    }

    auto augmentedMatrix(m_Matrix);
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

const std::vector<BigInteger> &Solver::DistributionCondQ(const BlockSet &set, Block blk, int &min)
{
    int dMines, dBlanks;
    BlockSet sets1;
    GetIntersectionCounts(set, sets1, dMines, dBlanks);
    auto length = 0;
    for (auto b : sets1)
        length += b;

    min = dMines;
    return DistCondQ(DistCondQParameters(sets1, m_SetIDs[blk], length));
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
                for (auto &id : m_SetIDs)
                    if (id > i)
                        --id;
                    else if (id == i)
                        id = it->second;
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
            throw;
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
            for (auto &id : m_SetIDs)
                if (id > col)
                    --id;
        }
    }
    return flag;
}

bool Solver::SimpleOverlapAll()
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
        {
            for (auto j = i + 1; j < indexes.size(); ++j)
            {
                auto rowRemoved = false;
                flag |= SimpleOverlap(indexes[i], indexes[j], rowRemoved);
                if (rowRemoved)
                    return true;
            }
        }
    }
    
    return flag;
}

bool Solver::SimpleOverlap(int r1, int r2, bool &rowRemoved)
{
    if (!m_Pairs.emplace(r1, r2).second)
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
    if (n1 == nullptr || n2 == nullptr)
        throw;

    if (exceptA.empty() && exceptB.empty())
    {
        if (n1->Value != n2->Value)
            throw;

        m_Matrix.RemoveRow(r2);
        rowRemoved = true;
        return false;
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

    //if (exceptA.empty() || exceptB.empty())
    //{
    //    Node<int> *nr;
    //    if (exceptA.empty())
    //    {
    //        n2->Value -= n1->Value;
    //        nr = m_Matrix.GetRowHead(r2).Right;
    //    }
    //    else
    //    {
    //        n1->Value -= n2->Value;
    //        nr = m_Matrix.GetRowHead(r1).Right;
    //    }
    //    auto it = intersection.begin();
    //    while (nr->Col != m_BlockSets.size())
    //    {
    //        if (nr->Col != *it)
    //        {
    //            nr = nr->Right;
    //            continue;
    //        }
    //        auto tmp = nr;
    //        nr = nr->Right;
    //        m_Matrix.Remove(*tmp);
    //        if (++it == intersection.end())
    //            break;
    //    }
    //}

    return flag;
}

std::vector<int> Gauss(OrthogonalList<int> &matrix)
{
    auto n = matrix.GetWidth();
    auto minorCol = std::vector<int>();
    auto major = 0;
    for (auto col = 0; col < n; ++col)
    {
        auto biasNode = matrix.SeekDown(major, col).Down;
        while (biasNode != nullptr && biasNode->Value == 0)
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

        auto vec = std::vector<std::pair<int, int>>();
        assert(abs(biasNode->Value) == 1);
        auto theBiasInv = biasNode->Value;
        auto node = matrix.GetColHead(col).Down;
        while (node != nullptr)
        {
            auto tmp = node->Down;
            if (node->Row != biasNode->Row)
            {
                vec.emplace_back(node->Row, -node->Value * theBiasInv);
                matrix.Remove(*node);
            }
            else
            {
                vec.emplace_back(node->Row, theBiasInv);
                node->Value = 1;
            }
            node = tmp;
        }

        auto bias = biasNode->Right;
        while (bias != nullptr)
        {
            auto biasVal = bias->Value;
            auto nc = &matrix.GetColHead(bias->Col);
            for (auto it = vec.begin(); it != vec.end(); ++it)
            {
                while (nc->Down != nullptr && nc->Down->Row < it->first)
                    nc = nc->Down;

                int oldV;
                if (nc->Down == nullptr ||
                    nc->Down->Row > it->first)
                    oldV = 0;
                else
                    oldV = nc->Down->Value;

                auto val = (it->first != biasNode->Row ? oldV : 0) + it->second * biasVal;

                if (nc->Down == nullptr ||
                    nc->Down->Row > it->first)
                {
                    if (abs(val) < 1E-14)
                        continue;

                    auto nr = &matrix.SeekRight(it->first, bias->Col);
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

void Solver::EnumerateSolutions(const std::vector<int> &minors, const OrthogonalList<int> &augmentedMatrix)
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
    auto sums = std::vector<int>(mR);
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
                    auto val = sums[mainRow];
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
    if (from.size() > to.size())
        throw;
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

void Solver::GetIntersectionCounts(const BlockSet &set1, std::vector<int> &sets1, int &mines, int &blanks) const
{
    auto lst1 = BlockSet(set1);
    sets1.clear();
    sets1.resize(m_BlockSets.size(), 0);

    for (auto blk : set1)
    {
        switch (m_Manager[blk])
        {
        case BlockStatus::Mine:
            mines++;
            break;
        case BlockStatus::Blank:
            blanks++;
            break;
        case BlockStatus::Unknown:
            ++sets1[m_SetIDs[blk]];
            break;
        default:
            break;
        }
    }
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
