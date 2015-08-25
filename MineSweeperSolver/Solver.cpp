#include "Solver.h"
#include "BinomialHelper.h"
#include <algorithm>

static void Overlap(const BlockSet &setA, const BlockSet &setB, BlockSet &exceptA, BlockSet &exceptB, BlockSet &intersection);
static std::vector<int> Gauss(OrthogonalList<double> &matrix);
static void Merge(const std::vector<BigInteger> &from, std::vector<BigInteger> &to);
static void Add(std::vector<BigInteger> &from, const std::vector<BigInteger> &cases);
static unsigned __int64 Hash(const BlockSet &set);

Solver::Solver(int count) : m_Manager(count, Unknown), m_Probability(count)
{
    auto lst = std::vector<Block>(count);
    for (auto i = 0; i < count; ++i)
        lst[i] = i;
    m_BlockSets.push_back(move(lst));
    m_Matrix.ExtendWidth(2);
}

BlockStatus Solver::GetBlockStatus(Block block) const
{
    return m_Manager[block];
}

double Solver::GetProbability(Block block) const
{
    return m_Probability[block];
}

const BigInteger &Solver::GetTotalStates() const
{
    return m_TotalStates;
}

void Solver::AddRestrain(Block blk, bool isMine)
{
    m_Manager[blk] = isMine ? Mine : Blank;
}

void Solver::AddRestrain(const BlockSet &set, int mines)
{
    auto theSet = BlockSet(set);
    sort(theSet.begin(), theSet.end());
    auto dMines = 0, dBlanks = 0;
    ReduceSet(theSet, dMines, dBlanks);

    m_Matrix.ExtendHeight(m_Matrix.GetHeight() + 1);

    auto col = 0;
    auto nr = m_Matrix.GetRowHead(m_Matrix.GetHeight() - 1);
    for (auto it = m_BlockSets.begin(); it != m_BlockSets.end(); ++it , ++col)
    {
        BlockSet exceptA, exceptB, intersection;
        Overlap(*it, theSet, exceptA, exceptB, intersection);
        if (intersection.empty())
            continue;

        if (exceptA.empty())
        {
            it->swap(intersection);
            nr = m_Matrix.Add(nr, m_Matrix.GetColHead(col), 1);
        }
        else
        {
            it->swap(exceptA);
            if (!intersection.empty())
            {
                m_Matrix.InsertCol(col + 1);
                auto node = m_Matrix.GetColHead(col).Down;
                auto nodeX = m_Matrix.GetColHead(col + 1);
                while (node != nullptr)
                {
                    nodeX = m_Matrix.Add(*node, nodeX, std::move(node->Value));
                    node = node->Down;
                }
                nr = m_Matrix.Add(nr, nodeX, 1);

                ++it , ++col;
                m_BlockSets.insert(it, BlockSet());
                it->swap(intersection);
            }
        }
        theSet.swap(exceptB);

        if (theSet.empty())
            break;
    }


    if (!theSet.empty())
    {
        m_BlockSets.push_back(BlockSet());
        m_BlockSets.back().swap(theSet);

        m_Matrix.ExtendWidth(m_Matrix.GetWidth() + 1);
        m_Matrix.Add(nr, m_Matrix.GetColHead(m_Matrix.GetWidth() - 1), 1);
    }

    m_Matrix.ExtendWidth(m_Matrix.GetWidth() + 1);
    m_Matrix.Add(nr, m_Matrix.GetColHead(m_Matrix.GetWidth() - 1), mines - dMines);
}

void Solver::Solve(bool withProb)
{
    m_Solutions.clear();

    auto flag = true;
    while (flag)
    {
        while (ReduceRestrains()) {}
        flag = SimpleOverlap();
    }

    if (!withProb)
        return;

    if (m_BlockSets.size() == 0)
    {
        m_TotalStates = BigInteger(1);

        m_Solutions.push_back(std::move(Solution(std::vector<int>())));
        ProcessSolutions();
        return;
    }

    auto augmentedMatrix = OrthogonalList<double>(m_Matrix);
    auto minors = Gauss(augmentedMatrix);

    if (!minors.empty() &&
        minors.back() == m_BlockSets.size())
        minors.erase(minors.end() - 1);
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

std::map<int, BigInteger> Solver::DistributionCond(const BlockSet& set, const BlockSet& setCond, int mines)
{
    int dMines, dBlanks;
    auto lst = BlockSet(set);
    ReduceSet(lst, dMines, dBlanks);

    int dMinesCond, dBlanksCond;
    auto lstCond = BlockSet(setCond);
    ReduceSet(lstCond, dMinesCond, dBlanksCond);

    if (lstCond.empty())
    {
        if (dMinesCond != mines)
            return std::map<int, BigInteger>();
    }

    BlockSet sets1, sets2, sets3;
    GetIntersectionCounts(lst, lstCond, sets1, sets2, sets3);

    auto dist = DistCond(DistCondParameters(sets1, sets2, sets3, mines - dMinesCond, lst.size()));
    auto dic = std::map<int, BigInteger>();
    for (auto i = 0; i < dist.size(); i++)
        dic.insert_or_assign(dic.end(), i + dMines, dist[i]);
    return dic;
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
                    m_Matrix.Add(*node, m_Matrix.GetColHead(col), std::move(node->Value));
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
    m_BlockSets.push_back(BlockSet());
    m_BlockSets.back().swap(theSet);

    return blkLst;
}

void Solver::ReduceSet(BlockSet &set, int &mines, int &blanks) const
{
    mines = 0;
    blanks = 0;
    for (auto it = set.begin(); it != set.end(); ++it)
        switch (m_Manager[*it])
        {
        case Mine:
            mines++;
            break;
        case Blank:
            blanks++;
            break;
        case Unknown:
        default:
            break;
        }
}

bool Solver::ReduceRestrains()
{
    auto flag = false;
    auto row = 0;
    auto n = m_Matrix.GetColHead(m_BlockSets.size()).Down;
    while (row >= m_Matrix.GetHeight())
    {
        if (n == nullptr || n->Row > row || n->Value == 0)
        {
            auto nr = m_Matrix.GetRowHead(row).Right;
            while (nr != nullptr)
            {
                auto col = nr->Col;
                std::for_each(m_BlockSets[col].begin(), m_BlockSets[col].end(), [this](Block &blk)
                              {
                                  m_Manager[blk] = Blank;
                              });
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
            while (nr != nullptr)
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
            while (nr != nullptr)
            {
                auto col = nr->Col;
                std::for_each(m_BlockSets[col].begin(), m_BlockSets[col].end(), [this](Block &blk)
                              {
                                  m_Manager[blk] = Mine;
                              });
                nr = nr->Right;
                flag = true;
            }
            if (n != nullptr && n->Row == row)
                n = n->Down;

            m_Matrix.RemoveRow(row);
            continue;
        }
        row++;
    }

    for (auto col = 0; col < m_BlockSets.size(); ++col)
    {
        int dMines, dBlanks;
        ReduceSet(m_BlockSets[col], dMines, dBlanks);
        if (dMines != 0)
        {
            auto nc = m_Matrix.GetColHead(col).Down;
            auto ncc = m_Matrix.GetColHead(m_BlockSets.size());
            while (nc != nullptr)
            {
                auto node = m_Matrix.SeekDown(nc->Row, ncc);
                if (node.Down == nullptr ||
                    node.Down->Row != nc->Row ||
                    (node.Down->Value -= dMines) < 0)
                {
                    m_TotalStates = BigInteger(0);
                    return true;
                }
                nc = nc->Down;
                ncc = *node.Down;
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
                flag |= SimpleOverlap(i, j);
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
            std::for_each(lst.begin(), lst.end(), [&iv, this](const int &index)
                          {
                              iv.second += m_BlockSets[index].size();
                          });
            return iv;
        };

    auto subs = [](Iv i1, Iv i2)-> Iv
        {
            return Iv(i2.first - i1.second, i2.second - i1.first);
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
                std::for_each(lst.begin(), lst.end(), [this](const int &id)
                              {
                                  std::for_each(m_BlockSets[id].begin(), m_BlockSets[id].end(), [this](const int &blk)
                                                {
                                                    m_Manager[blk] = Mine;
                                                });
                              });
                flag = true;
            }
            else if (iv0.first == iv.second)
            {
                std::for_each(lst.begin(), lst.end(), [this](const int &id)
                              {
                                  std::for_each(m_BlockSets[id].begin(), m_BlockSets[id].end(), [this](const int &blk)
                                                {
                                                    m_Manager[blk] = Blank;
                                                });
                              });
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
        auto biasNode = matrix.SeekDown(major + 1, col).Down;
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
            auto nc = matrix.GetColHead(bias->Col);
            for (auto row = 0; row < m; row++)
            {
                double oldV;
                if (nc.Down == nullptr ||
                    nc.Down->Row > row)
                    oldV = 0;
                else
                    oldV = nc.Down->Value;

                auto val = (row != biasNode->Row ? oldV : 0) + vec[row] * biasVal;

                if (nc.Down == nullptr ||
                    nc.Down->Row > row)
                {
                    if (abs(val) < 1E-14)
                        continue;

                    auto nr = matrix.SeekRight(row, bias->Col);

                    nc = matrix.Add(nr, nc, std::move(val));
                }
                else if (abs(val) < 1E-14)
                    matrix.Remove(*nc.Down);
                else
                {
                    nc = *nc.Down;
                    nc.Value = val;
                }
            }
            bias = bias->Right;
        }

        if (major != biasNode->Row)
        {
            auto majNode = matrix.GetRowHead(major);
            auto biaNode = matrix.GetRowHead(biasNode->Row);
            while (true)
            {
                auto majID = majNode.Right != nullptr ? majNode.Right->Col : n;
                auto biaID = biaNode.Right != nullptr ? biaNode.Right->Col : n;
                if (majID < biaID)
                {
                    biaNode = matrix.Add(biasNode->Row, majID, std::move(majNode.Right->Value));
                    matrix.Remove(*majNode.Right);
                }
                else if (majID > biaID)
                {
                    majNode = matrix.Add(major, biaID, std::move(biaNode.Right->Value));
                    matrix.Remove(*biaNode.Right);
                }
                else if (majNode.Right != nullptr)
                {
                    majNode = *majNode.Right;
                    biaNode = *biaNode.Right;
                    std::swap(biaNode.Value, majNode.Value);
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
        m_Solutions.push_back(Solution(move(lst)));
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
                    m_Solutions.push_back(Solution(move(lst)));
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
    auto total = BigInteger(0);
    std::for_each(m_Solutions.begin(), m_Solutions.end(), [&total,&exp,this](Solution &so)
                  {
                      so.States = BigInteger(1);
                      for (auto i = 0; i < m_BlockSets.size(); ++i)
                          so.States *= Binomial(m_BlockSets[i].size(), so.Dist[i]);
                      total += so.States;
                      for (auto i = 0; i < m_BlockSets.size(); ++i)
                      {
                          auto v = so.States * so.Dist[i];
                          exp[i] += v;
                      }
                  });
    std::for_each(m_Solutions.begin(), m_Solutions.end(), [&total, this](Solution &so)
                  {
                      so.Ratio = so.States / total;
                  });
    for (auto i = 0; i < m_Manager.size(); ++i)
        if (m_Manager[i] == Mine)
            m_Probability[i] = 1;
        else if (m_Manager[i] == Blank)
            m_Probability[i] = 0;

    for (auto i = 0; i < m_BlockSets.size(); ++i)
    {
        if (exp[i] == 0)
        {
            std::for_each(m_BlockSets[i].begin(), m_BlockSets[i].end(), [this](int &blk)
                          {
                              m_Manager[blk] = Blank;
                          });
        }
        else if (exp[i] == total * m_BlockSets[i].size())
        {
            std::for_each(m_BlockSets[i].begin(), m_BlockSets[i].end(), [this](int &blk)
                          {
                              m_Manager[blk] = Mine;
                          });
        }

        auto p = (exp[i] / total) / m_BlockSets[i].size();
        std::for_each(m_BlockSets[i].begin(), m_BlockSets[i].end(), [&p, this](int &blk)
                      {
                          m_Probability[blk] = p;
                      });
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

void Solver::GetIntersectionCounts(const BlockSet& set1, const BlockSet& set2, std::vector<int>& sets1, std::vector<int>& sets2, std::vector<int>& sets3) const
{
    auto lst1 = BlockSet(set1), lst2 = BlockSet(set2);
    sets1.clear();
    sets1.reserve(m_BlockSets.size());
    sets2.clear();
    sets2.reserve(m_BlockSets.size());
    sets3.clear();
    sets3.reserve(m_BlockSets.size());
    for (auto i = 0; i < m_BlockSets.size();++i)
    {
        BlockSet exceptA, exceptB1T, exceptB2T, exceptB1, exceptB2, exceptC, resume1, resume2;
        Overlap(m_BlockSets[i], lst1, exceptA, resume1, exceptB1T);
        Overlap(m_BlockSets[i], lst1, exceptA, resume1, exceptB1T);
        Overlap(exceptB1T, exceptB2T, exceptB1, exceptB2, exceptC);
        lst1.swap(resume1);
        lst2.swap(resume2);
        sets1[i] = exceptB1.size();
        sets2[i] = exceptB2.size();
        sets3[i] = exceptC.size();
    }
}

std::vector<BigInteger> Solver::DistCond(const DistCondParameters& par)
{
    auto &dic = m_DistCondCache[par];
    if (!dic.empty())
        return dic;

    dic.resize(par.Length + 1);
    std::for_each(m_Solutions.begin(), m_Solutions.end(), [&par, &dic, this](Solution &solution)
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
                std::for_each(stack.begin(), stack.end(), [&mns](int&v) { mns += v; });
                if (mns >= 0 &&
                    mns <= max[m_BlockSets.size() - 1])
                {
                    stack.push_back(mns);

                    auto dicT = std::vector<BigInteger>(1, BigInteger(1));
                    for (auto i = 0; i < m_BlockSets.size(); i++)
                    {
                        auto n = m_BlockSets[i].size();
                        auto a = par.Sets1[i];
                        auto b = par.Sets2[i];
                        auto c = par.Sets3[i];
                        auto m = solution.Dist[i]; // in a + b + c
                        auto p = stack[i]; // in b + c

                        {
                            auto cases = std::vector<BigInteger>();
                            cases.reserve(min(p, c) + 1);
                            for (auto j = 0; j <= p && j <= c; j++)
                                cases.push_back(Binomial(c, j) * Binomial(b, p - j));

                            Add(dicT, cases);
                        }
                        {
                            auto cases = std::vector<BigInteger>();
                            cases.reserve(min(m - p, a) + 1);
                            for (auto j = 0; j <= m - p && j <= a; j++)
                                cases.push_back(Binomial(a, j) * Binomial(n - a - b - c, m - p - j));

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
    });
    return dic;
}

DistCondParameters::DistCondParameters(const BlockSet& sets1, const BlockSet& sets2, const BlockSet& sets3, int minesCond, int length)
    : Sets1(sets1),
    Sets2(sets2),
    Sets3(sets3),
    MinesCond(minesCond),
    Length(length)
{
    m_Hash = (Hash(Sets1) << 44) ^ (Hash(Sets2) << 24) ^ (Hash(Sets3) << 4) ^ MinesCond;
}

bool operator==(const DistCondParameters& lhs, const DistCondParameters& rhs) 
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

bool operator!=(const DistCondParameters& lhs, const DistCondParameters& rhs) 
{
    return !(lhs == rhs);
}

bool operator<(const DistCondParameters& lhs, const DistCondParameters& rhs) 
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

unsigned __int64 Hash(const BlockSet &set)
{
    unsigned __int64 hash = 5381;
    auto it = set.begin();
    while (it != set.end())
        hash = (hash << 5) + hash + *it++ + 30;
    return hash;
}
