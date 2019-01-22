#include "BasicDrainer.h"

static void Combinations(int n, int m, std::vector<std::vector<BlockStatus>> &dists)
{
    auto count = 0;
    std::vector<BlockStatus> stack;
    stack.reserve(n);
    auto add = [&count, &stack, n, m]()
        {
            if (count < m)
            {
                if (stack.size() < n - 1)
                {
                    auto b = m - count >= n - stack.size();
                    stack.push_back(b ? BlockStatus::Mine : BlockStatus::Blank);
                    if (b)
                        ++count;
                    return false;
                }
                if (stack.empty())
                    return true;
                if (stack.back() == BlockStatus::Mine)
                {
                    while (!stack.empty() && stack.back() == BlockStatus::Mine)
                    {
                        stack.pop_back();
                        --count;
                    }
                    if (stack.empty())
                        return true;
                }
                stack.back() = BlockStatus::Mine;
                ++count;
                return false;
            }
            if (stack.size() < n - 1)
            {
                stack.push_back(BlockStatus::Blank);
                return false;
            }
            while (true)
            {
                while (!stack.empty() && stack.back() == BlockStatus::Mine)
                {
                    stack.pop_back();
                    --count;
                }
                if (stack.empty())
                    return true;
                if (count < m)
                {
                    stack.back() = BlockStatus::Mine;
                    ++count;
                    return false;
                }
                while (!stack.empty() && stack.back() == BlockStatus::Blank)
                    stack.pop_back();
                if (stack.empty())
                    return true;
            }
        };
    while (true)
    {
        if (stack.size() < n - 1)
        {
            if (add())
                break;
            continue;
        }
        ASSERT(count == m - 1 || count == m);
        stack.push_back(count == m - 1 ? BlockStatus::Mine : BlockStatus::Blank);
        dists.push_back(stack);

        stack.pop_back();
        if (add())
            break;
    }
}

MacroSituation::MacroSituation() : m_ToOpen(0), m_Solver(nullptr), m_BestProb(NAN), m_Hash(Hash()) {}

MacroSituation::MacroSituation(const MacroSituation &other) : m_ToOpen(other.m_ToOpen), m_Solver(nullptr), m_Degrees(other.m_Degrees), m_BestProb(NAN), m_Hash(other.m_Hash)
{
    if (other.m_Solver != nullptr)
#ifdef USE_BASIC_SOLVER
        m_Solver = new BasicSolver(*other.m_Solver);
#else
        m_Solver = new Solver(*other.m_Solver);
#endif
}

MacroSituation::~MacroSituation()
{
    if (m_Solver == nullptr)
        return;
    delete m_Solver;
    m_Solver = nullptr;
}

size_t MacroSituation::Hash()
{
    if (m_Degrees.empty())
        return m_Hash = m_BestProb < 0.5 ? 0x0000000000000000 : 0xffffffffffffffff;

    size_t hash = 5381;
    for (auto d : m_Degrees)
        hash = (hash << 5) + hash + d;

    return m_Hash = hash;
}

bool operator==(const MacroSituation &lhs, const MacroSituation &rhs)
{
    if (lhs.m_Hash != rhs.m_Hash)
        return false;
    if (lhs.m_Degrees.empty() || rhs.m_Degrees.empty())
        return lhs.m_Solver == rhs.m_Solver;
    //if (lhs.m_Degrees != rhs.m_Degrees)
    //    return false;
    return true;
}

bool operator!=(const MacroSituation &lhs, const MacroSituation &rhs)
{
    return !(lhs == rhs);
}

BasicDrainer::~BasicDrainer()
{
    for (auto &kvp : m_Macros)
    {
        if (kvp.second == nullptr)
            continue;
        delete kvp.second;
        kvp.second = nullptr;
    }
}

BasicDrainer::BasicDrainer() : m_RootMacro(nullptr)
{
    {
        auto macro = new MacroSituation();
        macro->m_BestProb = 1;
        macro->Hash();
        m_SucceedMacro = GetOrAddMacroSituation(macro);
    }
    {
        auto macro = new MacroSituation();
        macro->m_BestProb = 0;
        macro->Hash();
        m_FailMacro = GetOrAddMacroSituation(macro);
    }
}

double BasicDrainer::GetBestProb() const
{
    return m_RootMacro->m_BestProb;
}

void BasicDrainer::Drain()
{
    for (auto &micro : m_Micros)
        SolveMicro(micro, m_RootMacro);

    std::vector<MacroSituation *> macros;
    for (auto macro : m_Macros)
        macros.push_back(macro.second);

    for (auto i = 0; i < macros.size(); ++i)
        for (auto j = i; j < macros.size(); ++j)
        {
            auto flag = true;
            for (auto ma : macros[j]->m_Incomings)
            {
                int k;
                for (k = 0; k < i; ++k)
                    if (macros[k] == ma)
                        break;
                if (k == i)
                {
                    flag = false;
                    break;
                }
            }
            if (flag)
            {
                if (i == j)
                    break;
                auto t = macros[i];
                macros[i] = macros[j];
                macros[j] = t;
                break;
            }
        }
    ASSERT(macros.back() == m_RootMacro);

    for (auto ma : macros)
    {
        if (ma->m_Degrees.empty())
            continue;

        ma->m_BestProb = 0;
        auto &probs = ma->m_Probs;
        probs.resize(m_BlocksR.size(), -1);
        for (auto i = 0; i < m_BlocksR.size(); ++i)
        {
            if (ma->m_Degrees[i] >= 0 || ma->m_Solver->GetBlockStatus(i) != BlockStatus::Unknown)
                continue;
            double prob = 0;
            for (auto kvp : ma->m_Transfer[i])
                prob += kvp.second->m_BestProb;
            prob /= ma->m_Transfer[i].size();
            probs[i] = prob;
            if (prob > ma->m_BestProb)
                ma->m_BestProb = prob;
        }

        for (auto i = 0; i < m_BlocksR.size(); ++i)
            if (probs[i] > ma->m_BestProb - 1E-6)
                ma->m_BestBlocks.push_back(i);
    }
}

void BasicDrainer::Update(MacroSituation *&macro)
{
    auto newRoot = GetOrAddMacroSituation(macro);
    ASSERT(macro == nullptr);
    ASSERT(newRoot != m_SucceedMacro);
    ASSERT(newRoot != m_FailMacro);
    m_RootMacro = newRoot;
}

MacroSituation *BasicDrainer::GetOrAddMacroSituation(MacroSituation *&macro)
{
    auto hash = macro->m_Hash;
    auto itp = m_Macros.equal_range(hash);
    for (auto it = itp.first; it != itp.second; ++it)
    {
        if (*it->second == *macro)
        {
            if (macro == m_SucceedMacro || macro == m_FailMacro)
                return macro;

            delete macro;
            macro = nullptr;
            return it->second;
        }
    }
    m_Macros.insert(std::make_pair(macro->m_Hash, macro));
    return macro;
}

void BasicDrainer::GenerateMicros(const std::vector<BlockSet> &sets, size_t totalStates, const std::vector<Solution> &solutions)
{
    m_Micros.reserve(totalStates);

    std::vector<std::map<int, std::vector<std::map<int, BlockStatus>>>> dicc(sets.size());
    std::vector<std::vector<std::map<int, BlockStatus>>*> ddic;
    std::vector<std::vector<BlockStatus>> dists;
    std::vector<int> stack;
    for (auto solution : solutions)
    {
        ddic.clear();
        for (auto i = 0; i < sets.size(); ++i)
        {
            auto m = solution.Dist[i];
            auto &lst = dicc[i][m];
            if (lst.empty())
            {
                dists.clear();
                Combinations(sets[i].size(), m, dists);
                for (auto l : dists)
                {
                    lst.emplace_back();
                    auto &d = lst.back();
                    for (auto j = 0; j < sets[i].size(); ++j)
                        d.insert(std::make_pair(sets[i][j], l[j]));
                }
            }
            ddic.push_back(&lst);
        }

        stack.clear();
        stack.reserve(sets.size());
        stack.push_back(0);
        while (true)
            if (stack.size() == sets.size())
                if (stack.back() < ddic[stack.size() - 1]->size())
                {
                    m_Micros.emplace_back();
                    auto &lst = m_Micros.back();
                    lst.resize(m_BlocksR.size(), BlockStatus::Blank);
                    for (auto i = 0; i < stack.size(); ++i)
                        for (auto kvp : ddic[i]->at(stack[i]))
                            lst[kvp.first] = kvp.second;

                    ++stack.back();
                }
                else
                {
                    stack.pop_back();
                    if (stack.empty())
                        break;
                    ++stack.back();
                }
            else if (stack.back() < ddic[stack.size() - 1]->size())
                stack.push_back(0);
            else
            {
                stack.pop_back();
                if (stack.empty())
                    break;
                ++stack.back();
            }
    }
}

#ifdef USE_BASIC_SOLVER
void BasicDrainer::GenerateRoot(BasicSolver *solver, int toOpen)
#else
void BasicDrainer::GenerateRoot(Solver *solver, int toOpen)
#endif
{
    auto macro = new MacroSituation();
    macro->m_Degrees.resize(m_BlocksR.size(), -1);
    macro->m_ToOpen = toOpen;
    macro->m_Solver = solver;
    macro->Hash();
    m_RootMacro = GetOrAddMacroSituation(macro);
}

void BasicDrainer::SolveMicro(MicroSituation &micro, MacroSituation *macro)
{
    macro->m_Micros.insert(&micro);

    BlockSet bests;
    for (auto i = 0; i < macro->m_Degrees.size(); ++i)
        if (macro->m_Degrees[i] == -1 && macro->m_Solver->GetBlockStatus(i) == BlockStatus::Unknown)
            bests.push_back(i);

    HeuristicPruning(macro, bests);

    for (auto i : bests)
    {
        auto ma = SolveMicro(micro, macro, i);
        auto maa = GetOrAddMacroSituation(ma);

        auto &map = macro->m_Transfer[i];
        auto res = map.insert(std::make_pair(&micro, maa));
        if (!res.second)
        {
            ASSERT(res.first->second == maa);
            continue;
        }

        macro->m_Incomings.insert(maa);

        if (maa->m_Degrees.empty())
            continue;

        SolveMicro(micro, maa);
    }
}

MacroSituation *BasicDrainer::SolveMicro(MicroSituation &micro, MacroSituation *macroOld, Block blk)
{
    if (micro[blk] == BlockStatus::Mine)
        return m_FailMacro;

    auto macro = new MacroSituation(*macroOld);
    OpenBlock(micro, macro, blk);
    if (macro->m_ToOpen == 0)
    {
        delete macro;
        return m_SucceedMacro;
    }

    while (true)
    {
        if (macro->m_Solver->CanOpenForSure == 0)
            macro->m_Solver->Solve(SolvingState::Reduce | SolvingState::Overlap | SolvingState::Probability, true);

        if (macro->m_Solver->CanOpenForSure == 0)
            return macro;

        for (auto i = 0; i < macro->m_Degrees.size(); ++i)
        {
            if (macro->m_Degrees[i] >= 0 || macro->m_Solver->GetBlockStatus(i) != BlockStatus::Blank)
                continue;
            OpenBlock(micro, macro, i);
            if (macro->m_ToOpen == 0)
            {
                delete macro;
                return m_SucceedMacro;
            }
        }
    }
}

void BasicDrainer::OpenBlock(MicroSituation &micro, MacroSituation *macro, Block blk)
{
    if (macro->m_Degrees[blk] >= 0)
        return;
    ASSERT(micro[blk] == BlockStatus::Blank);
    if (macro->m_Solver->GetBlockStatus(blk) == BlockStatus::Blank && macro->m_Degrees[blk] != -127)
        --macro->m_Solver->CanOpenForSure;
    macro->m_Solver->AddRestrain(blk, false);
    auto degree = 0;
    for (auto b : m_BlocksR[blk])
        if (micro[b] == BlockStatus::Mine)
            ++degree;
    macro->m_Degrees[blk] = degree;
    if (degree == 0)
        for (auto b : m_BlocksR[blk])
            OpenBlock(micro, macro, b);
    else
        macro->m_Solver->AddRestrain(m_BlocksR[blk], degree);

    --macro->m_ToOpen;
    macro->Hash();
}
