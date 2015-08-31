#include "Drainer.h"

#ifdef _DEBUG
#define ASSERT(val) if (!(val)) throw;
#else
#define ASSERT(val)
#endif

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

MacroSituation::MacroSituation(const MacroSituation &other) : m_Degrees(other.m_Degrees), m_ToOpen(other.m_ToOpen), m_Solver(nullptr), m_BestProb(NAN), m_Hash(other.m_Hash)
{
    if (other.m_Solver != nullptr)
        m_Solver = new Solver(*other.m_Solver);
}

MacroSituation::~MacroSituation()
{
    if (m_Solver == nullptr)
        return;
    delete m_Solver;
    m_Solver = nullptr;
}

unsigned __int64 MacroSituation::Hash()
{
    if (m_Degrees.empty())
        return m_Hash = m_BestProb < 0.5 ? 0x0000000000000000 : 0xffffffffffffffff;

    unsigned __int64 hash = 5381;
    for (auto d : m_Degrees)
        hash = (hash << 5) + hash + d;

    return m_Hash = hash;
}

Drainer::Drainer(const GameMgr &mgr) : m_Mgr(mgr)
{
    for (auto i = 0; i < m_Mgr.m_Blocks.size(); ++i)
    {
        if (m_Mgr.m_Blocks[i].IsOpen || m_Mgr.m_Solver->GetBlockStatus(i) != BlockStatus::Unknown)
            continue;
        m_BlocksLookup.insert(std::make_pair(i, m_Blocks.size()));
        m_Blocks.push_back(i);
    }
    m_BlocksR.resize(m_Blocks.size());
    m_DMines.resize(m_Blocks.size(), 0);
    for (auto i = 0; i < m_Blocks.size(); ++i)
    {
        for (auto blk : m_Mgr.m_BlocksR[m_Blocks[i]])
            switch (m_Mgr.m_Solver->m_Manager[blk])
            {
            case BlockStatus::Unknown:
                m_BlocksR[i].push_back(m_BlocksLookup[blk]);
                break;
            case BlockStatus::Mine:
                ++m_DMines[i];
                break;
            case BlockStatus::Blank:
                break;
            default:
                ASSERT(false);
                break;
            }
    }

    GenerateMicros();

    {
        auto macro = new MacroSituation();
        macro->m_Degrees.resize(m_Blocks.size(), -1);
        macro->m_ToOpen = m_Mgr.m_ToOpen;
        macro->m_Solver = new Solver(m_Blocks.size());
#ifdef _DEBUG
        macro->m_Solver->m_SetIDs.clear();
        macro->m_Solver->m_SetIDs.resize(m_Blocks.size(), -1);
#endif
        macro->m_Solver->m_BlockSets.clear();
        macro->m_Solver->m_BlockSets.reserve(m_Mgr.m_Solver->m_BlockSets.size());
        for (auto &set : m_Mgr.m_Solver->m_BlockSets)
        {
            macro->m_Solver->m_BlockSets.emplace_back();
            auto &setC = macro->m_Solver->m_BlockSets.back();
            setC.reserve(set.size());
            for (auto blk : set)
            {
                auto blkC = m_BlocksLookup[blk];
#ifdef _DEBUG
                if (blkC == 0)
                    ASSERT(m_Blocks[0] == blk);
#endif
                setC.push_back(blkC);
                ASSERT(macro->m_Solver->m_SetIDs[blkC] == -1);
                macro->m_Solver->m_SetIDs[blkC] = macro->m_Solver->m_BlockSets.size() - 1;
                ASSERT(&setC == &macro->m_Solver->m_BlockSets[macro->m_Solver->m_SetIDs[blkC]])
            }
        }
        macro->m_Solver->m_Matrix = m_Mgr.m_Solver->m_Matrix;
        macro->Hash();
        m_RootMacro = GetOrAddMacroSituation(macro);
    }
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
        probs.resize(m_Blocks.size(), -1);
        for (auto i = 0; i < m_Blocks.size(); ++i)
        {
            if (ma->m_Degrees[i] >= 0 || ma->m_Solver->m_Manager[i] != BlockStatus::Unknown)
                continue;
            double prob = 0;
            for (auto kvp : ma->m_Transfer[i])
                prob += kvp.second->m_BestProb;
            prob /= ma->m_Transfer[i].size();
            probs[i] = prob;
            if (prob > ma->m_BestProb)
                ma->m_BestProb = prob;
        }

        for (auto i = 0; i < m_Blocks.size(); ++i)
            if (probs[i] > ma->m_BestProb - 1E-6)
                ma->m_BestBlocks.push_back(i);
    }
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

Drainer::~Drainer()
{
    for (auto &kvp : m_Macros)
    {
        if (kvp.second == nullptr)
            continue;
        delete kvp.second;
        kvp.second = nullptr;
    }
}

double Drainer::GetBestProb() const
{
    return m_RootMacro->m_BestProb;
}

BlockSet Drainer::GetBestBlocks() const
{
    BlockSet set;
    set.reserve(m_RootMacro->m_BestBlocks.size());
    for (auto blk : m_RootMacro->m_BestBlocks)
        set.push_back(m_Blocks[blk]);
    return set;
}

const double *Drainer::GetBestProbabilities() const
{
    return &*m_Prob.begin();
}

void Drainer::Update()
{
    auto macro = new MacroSituation();
    macro->m_Degrees = m_RootMacro->m_Degrees;
    for (auto i = 0; i < m_Blocks.size(); ++i)
        if (m_Mgr.m_Blocks[m_Blocks[i]].IsOpen)
            macro->m_Degrees[i] = m_Mgr.m_Blocks[m_Blocks[i]].Degree - m_DMines[i];
    macro->Hash();
    auto newRoot = GetOrAddMacroSituation(macro);
    ASSERT(macro == nullptr);
    ASSERT(newRoot != m_SucceedMacro);
    ASSERT(newRoot != m_FailMacro);
    m_RootMacro = newRoot;

    m_Prob.clear();
    m_Prob.resize(m_Mgr.m_Blocks.size(), -1);
    for (auto i = 0; i < m_Blocks.size(); ++i)
        m_Prob[m_Blocks[i]] = m_RootMacro->m_Probs[i];
    for (auto i = 0; i < m_Mgr.m_Blocks.size(); ++i)
        switch (m_Mgr.m_Solver->m_Manager[i])
        {
        case BlockStatus::Unknown:
            ASSERT(m_Prob[i] >= 0 && m_Prob[i] <= 1);
            break;
        case BlockStatus::Mine:
            ASSERT(m_Prob[i] == -1);
            m_Prob[i] = 0;
            break;
        case BlockStatus::Blank:
            ASSERT(m_Prob[i] == -1);
            m_Prob[i] = 1;
            break;
        default:
            ASSERT(false);
        }
}

MacroSituation *Drainer::GetOrAddMacroSituation(MacroSituation *&macro)
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

void Drainer::GenerateMicros()
{
    m_Micros.reserve(static_cast<size_t>(m_Mgr.m_Solver->m_TotalStates));

    auto &sets = m_Mgr.m_Solver->m_BlockSets;
    std::vector<BlockSet> indexes;
    indexes.reserve(sets.size());
    for (auto &set : sets)
    {
        indexes.emplace_back();
        auto &lst = indexes.back();
        for (auto blk : set)
            lst.push_back(m_BlocksLookup[blk]);
    }
    std::vector<std::map<int, std::vector<std::map<int, BlockStatus>>>> dicc(sets.size());
    for (auto solution : m_Mgr.m_Solver->m_Solutions)
    {
        std::vector<std::vector<std::map<int, BlockStatus>>*> ddic;
        for (auto i = 0; i < sets.size(); i++)
        {
            auto m = solution.Dist[i];
            auto &lst = dicc[i][m];
            if (lst.empty())
            {
                std::vector<std::vector<BlockStatus>> dists;
                Combinations(sets[i].size(), m, dists);
                for (auto l : dists)
                {
                    lst.emplace_back();
                    auto &d = lst.back();
                    for (auto j = 0; j < sets[i].size(); ++j)
                        d.insert(std::make_pair(indexes[i][j], l[j]));
                }
            }
            ddic.push_back(&lst);
        }

        std::vector<int> stack;
        stack.reserve(sets.size());
        stack.push_back(0);
        while (true)
            if (stack.size() == sets.size())
                if (stack.back() < ddic[stack.size() - 1]->size())
                {
                    m_Micros.emplace_back();
                    auto &lst = m_Micros.back();
                    lst.resize(m_Blocks.size(), BlockStatus::Blank);
                    for (auto i = 0; i < stack.size(); ++i)
                        for (auto kvp : ddic[i]->at(stack[i]))
                            lst[kvp.first] = kvp.second;
#ifdef _DEBUG
					for (auto row = 0; row < m_Mgr.m_Solver->m_Matrix.GetHeight(); ++row)
					{
						auto v = 0;
						auto nr = m_Mgr.m_Solver->m_Matrix.GetRowHead(row).Right;
						ASSERT(nr != nullptr);
						while (nr->Col != m_Mgr.m_Solver->m_BlockSets.size())
						{
							for (auto blk : m_Mgr.m_Solver->m_BlockSets[nr->Col])
								if (lst[m_BlocksLookup[blk]] == BlockStatus::Mine)
									++v;
							nr = nr->Right;
							ASSERT(nr != nullptr);
						}
						ASSERT(nr->Value == v);
					}
#endif

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

void Drainer::SolveMicro(MicroSituation &micro, MacroSituation *macro)
{
    macro->m_Micros.insert(&micro);

    for (auto i = 0; i < macro->m_Degrees.size(); i++)
    {
        if (macro->m_Degrees[i] != -1 ||
            macro->m_Solver->m_Manager[i] != BlockStatus::Unknown)
            continue;

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

MacroSituation *Drainer::SolveMicro(MicroSituation &micro, MacroSituation *macroOld, Block blk)
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
        ASSERT((macro->m_Solver->GetSolvingState() & SolvingState::CanOpenForSure) == SolvingState::Stale);
        macro->m_Solver->Solve(SolvingState::Reduce | SolvingState::Overlap | SolvingState::Probability, true);
        if ((macro->m_Solver->GetSolvingState() & SolvingState::CanOpenForSure) == SolvingState::Stale)
#ifdef _DEBUG
		{
			for (auto i = 0; i < macro->m_Degrees.size(); ++i)
				if (macro->m_Degrees[i] < 0 && macro->m_Solver->m_Manager[i] == BlockStatus::Blank)
					throw;
			return macro;
		}
#else
            return macro;
#endif

#ifdef _DEBUG
		auto flag = false;
#endif
        for (auto i = 0; i < macro->m_Degrees.size(); ++i)
        {
            if (macro->m_Degrees[i] >= 0 || macro->m_Solver->m_Manager[i] != BlockStatus::Blank)
                continue;
            OpenBlock(micro, macro, i);
#ifdef _DEBUG
            flag = true;
#endif
            if (macro->m_ToOpen == 0)
            {
                delete macro;
                return m_SucceedMacro;
            }
        }
        ASSERT(flag);
    }
}

void Drainer::OpenBlock(MicroSituation &micro, MacroSituation *macro, Block blk)
{
    if (macro->m_Degrees[blk] >= 0)
        return;
    ASSERT(micro[blk] == BlockStatus::Blank);
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
