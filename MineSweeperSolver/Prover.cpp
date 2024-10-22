#include "BinomialHelper.h"
#include "facade.hpp"
#include "Prover.h"
#include "GameMgr.h"
#include "Util.h"
#include <mimalloc-new-delete.h>
#include <fmt/ostream.h>
#include <fmt/ranges.h>
#include <iterator>
#include <sys/sysinfo.h>
#include <condition_variable>
#include <sstream>
#include <stdexcept>
#include <thread>
#include <ranges>

#ifndef TRACEBACK
#define Traceback ""
#endif

Trie g_Trie{};
Strategy g_Strategy;
RCase g_InvalidCase;
static constexpr auto HEUR = SolvingState::Reduce | SolvingState::Overlap | SolvingState::Probability | SolvingState::Heuristic;

CaseRegistry g_Registry{};
std::atomic<double> g_MemoryAvailPercent;

void updateMemoryAvailPercent()
{
    std::ifstream fin("/proc/meminfo");
    size_t total, avail;
    std::string s;
    fin >> s >> total >> s;
    fin >> s >> avail >> s;
    fin >> s >> avail >> s;
    g_MemoryAvailPercent.store(100.0 * avail / total);
}

node_t *Trie::find(FCase c, int special)
{
    auto ptr = &root;
    for (auto id = 0; id <= c->LargestModifiedIndex; id++)
    {
        auto blk = c->Game().GetBlockProperties()[id];
        auto degree = id == c->Id ? special : blk.IsOpen ? blk.Degree : 9;
        ptr = ensure(ptr, degree);
    }
    return ptr;
}

node_t *Trie::ensure(node_t *ptr, int d)
{
    if (d < 0 || d >= ptr->next.size())
        throw std::logic_error{ "Index out of bound" };
    auto &nxt = ptr->next[d];
    auto next = nxt.load(std::memory_order_acquire);
    if (next)
        return next;

    std::lock_guard lock{ ptr->mtx };
    if ((next = nxt.load(std::memory_order_relaxed)))
        return next;

    next = new node_t{};
    ++cnt;
    nxt.store(next, std::memory_order_release);
    return next;
}

BaseCase::BaseCase(PCase p)
    : TotalStates{ p->TotalStates },
      Depth{ p->Depth },
      Step{ p->Step },
      Duplication{},
      LargestModifiedIndex{ p->LargestModifiedIndex },
      RegistryNext{},
      QueueNext{},
      m_Game{ p->m_Game } { }

BaseCase::BaseCase(PCase p, PGame game)
    : TotalStates{ game->GetSolver().GetTotalStates() },
      Depth{ p ? p->Depth : 0u },
      Step{ p ? p->Step : 0u },
      Duplication{},
      LargestModifiedIndex{ p ? p->LargestModifiedIndex : 0 },
      RegistryNext{},
      QueueNext{},
      m_Game{ std::move(game) } { }

BaseCase::~BaseCase() = default;

PGame BaseCase::ThePGame()
{
    if (std::holds_alternative<PGame>(m_Game))
        return std::get<PGame>(m_Game);

    if (std::holds_alternative<std::string>(m_Game))
    {
        std::stringstream ss{ std::get<std::string>(std::move(m_Game)) };
        m_Game = std::make_shared<GameMgr>(ss, &g_Strategy);
        return std::get<PGame>(m_Game);
    }

#ifdef TRACEBACK
    fmt::print("ERROR: Attempts to get depleted:\n");
    PrintTraceback();
#endif

    throw std::logic_error{ "Already depleted" };
}

PCase BaseCase::CheckedFork()
{
#ifdef TRACEBACK
    try
    {
        return Fork();
    }
    catch (const std::exception &err)
    {
        fmt::print("Error: {}\n",
                err.what());
        PrintTraceback();
        throw;
    }
#else
    return Fork();
#endif
}

#ifdef TRACEBACK
void BaseCase::PrintTraceback() const
{
    fmt::print("Traceback::\n");
    for (auto ptr = this; ptr; ptr = ptr->GetAnyParent())
        fmt::print("At {}\n",
                ptr->ToString());
    fmt::print("=======\n");
}
#endif

BaseCase &BaseCase::Deflate()
{
    if (std::holds_alternative<std::string>(m_Game))
        return *this;

    std::stringstream ss;
    std::get<PGame>(m_Game)->Save(ss);
    m_Game = ss.str();
    return *this;
}

void HolderCase::AddChildren(ACase v)
{
    v->Sibling = Child;
    Child = v;
}

void ReportingCase::AssignParent(FCase p)
{
    std::shared_lock lock{ ParentsMtx };
    AllParents.push_back(p);
}

PCase ForkedCase::Fork()
{
    auto [lb, ub] = Game().GetDegreeBounds(Id);
    for (; m_Degree <= ub; m_Degree++)
    {
        if (m_Degree < lb)
            continue;

        auto node = g_Trie.find(this, m_Degree);
        RCase c;
        if ((c = node->p.load(std::memory_order_acquire)))
            goto child;

        {
            std::lock_guard lock{ node->mtx };
            if ((c = node->p.load(std::memory_order_relaxed)))
                goto child;

            auto g = std::make_shared<GameMgr>(Game());
            g->SetBlockDegree(Id, m_Degree);
            g->Solve(HEUR, false);
            if (!g->GetStarted() // infeasible
                    || g->GetSolver().GetTotalStates() == 1) // guaranteed win
            {
                node->p.store(g_InvalidCase, std::memory_order_relaxed);
                continue;
            }

            if (g->GetBestBlockCount())
                c = new SafeCase(this, g, LargestModifiedIndex);
            else
                c = new UnsafeCase(this, g, LargestModifiedIndex);
            c->AssignParent(this);
#ifdef TRACEBACK
            c->operator PCase()->Traceback = Traceback + fmt::format("[{}]={}", Id, m_Degree);
#endif
            node->p.store(c, std::memory_order_release);
            m_Degree++;
            return c->operator PCase();
        }
child:
        if (c == g_InvalidCase)
            continue;
        c->AssignParent(this);
        // note that we shouldn't report c to main queue
#ifndef NDEBUG
        fmt::print("DUPLICATION on {} (@{})\n",
            c->operator PCase()->ToString(),
            fmt::ptr(c->operator PCase()));
#endif
    }
    return nullptr;
}

ActionCase::ActionCase(HCase p, PGame g, int id)
    : ForkedCase{ p, g, id },
      IntrinsicDanger{ Game().GetBlockProbability(id) * TotalStates },
      Danger{ IntrinsicDanger },
#ifdef TRACEBACK
      Parent{ p },
#endif
      Sibling{}
{
    ++Depth, ++Step;
}

PCase ActionCase::Fork()
{
    if (!m_Degree)
    {
        auto g = std::make_shared<GameMgr>(Game());
        g->SetBlockMine(Id, false);
        g->Solve(HEUR, false);
        if (!g->GetStarted()) // infeasible at all
            throw std::logic_error{ "All ActionCase should be feasible" };
        if (g->GetSolver().GetTotalStates() == 1)
            return nullptr; // guaranteed win
        m_Game = g;
    }

    return ForkedCase::Fork();
}

SafeCase::SafeCase(PCase p, PGame g, int lmi)
    : ForkedCase{ p, g, g->GetBestBlockList().front() }
{
    LargestModifiedIndex = std::max(LargestModifiedIndex, lmi);
}

UnsafeCase::UnsafeCase(PCase p, PGame g, int lmi)
    : HolderCase{ p, g, g->GetMinProbability() * TotalStates },
      m_List{ std::move(const_cast<BlockSet &>(Game().GetPreferredBlockList())) },
      m_It{ m_List.begin() }
{
    LargestModifiedIndex = lmi;
    Duplication = g->GetPreferredBlockCount();
}

void UnsafeCase::ResolveParents()
{
    m_CachedParents.assign(AllParents.begin(), AllParents.end());
    AllParents.clear();
}

PCase UnsafeCase::Fork()
{
    while (m_It != m_List.end())
    {
        auto ac = new ActionCase(this, ThePGame(), *m_It++);
        AddChildren(ac);
        return ac;
    }

    return nullptr;
}

void UnsafeCase::ReportDanger()
{
    ResolveDanger();

#ifndef NDEBUG
    fmt::print("{} ==> {}\n",
        ToString(),
        AllParents | std::views::transform([](FCase c) { return fmt::ptr(c); }));
#endif
    for (auto ac : AllParents)
        ac->Danger += Danger;
}

void HolderCase::ResolveDanger()
{
    auto ac = Child.load(std::memory_order_acquire);
    if (!ac) // not yet forked; report the intrinsic danger
        return;

#ifndef NDEBUG
    std::vector<std::string> tmp;
#endif
    auto d = std::numeric_limits<decltype(Danger)>::infinity();
    for (; ac; ac = ac->Sibling)
    {
        d = std::min(d, ac->Danger);
#ifndef NDEBUG
        tmp.push_back(fmt::format("@{}D{}", fmt::ptr(ac), ac->Danger));
#endif
    }
    Danger = d;
#ifndef NDEBUG
    fmt::print("{} <== {}\n",
        ToString(),
        fmt::join(tmp, ";"));
#endif
}

std::string BaseCase::ToString() const
{
    if (!std::holds_alternative<PGame>(m_Game))
        return fmt::format("[d{}s{}i{} TS{}]",
                Depth,
                Step,
                LargestModifiedIndex,
                TotalStates);
    return fmt::format("[d{}s{}i{} G={} TS{}]",
            Depth,
            Step,
            LargestModifiedIndex,
            fmt::ptr(std::get<PGame>(m_Game).get()),
            TotalStates);
}

std::string ForkedCase::ToString() const
{
    return fmt::format("{}@{}",
            BaseCase::ToString(),
            Id);
}

std::string HolderCase::ToString() const
{
    std::vector<double> tmp;
    for (auto c = Child.load(); c; c = c->Sibling)
        tmp.push_back(c->Danger);
    return fmt::format("{}[{:3g}]",
            BaseCase::ToString(),
            fmt::join(tmp, " "));
}

std::string ActionCase::ToString() const
{
    return fmt::format("Action{}",
            ForkedCase::ToString());
}

std::string SafeCase::ToString() const
{
    if (!std::holds_alternative<PGame>(m_Game))
        return fmt::format("Safe{}~{}:P{}",
                ForkedCase::ToString(),
                Traceback,
                AdditionalParents | std::views::transform([](FCase c) { return fmt::ptr(c); }));
    return fmt::format("Safe{}~{}:S{}P{}",
            ForkedCase::ToString(),
            Traceback,
            std::get<PGame>(m_Game)->GetBestBlockCount(),
            AdditionalParents | std::views::transform([](FCase c) { return fmt::ptr(c); }));
}

std::string UnsafeCase::ToString() const
{
    return fmt::format("Unsafe{}~{}:D{}P{}",
            HolderCase::ToString(),
            Traceback,
            Duplication,
            AdditionalParents | std::views::transform([](FCase c) { return fmt::ptr(c); }));
}

void CaseRegistry::Save(ACase ac)
{
    ++m_ACases;
    updateMax(m_MaxStep, ac->Step);
    push(m_ActionCases, ac);
}

void CaseRegistry::Save(SCase sc)
{
    ++m_SCases;
    boost::shared_lock lock{ m_Mutex };
    if (sc->Depth != m_MaxDepth)
        throw std::logic_error{ "Depth not matching" };
    push(m_D1SafeCases, sc);
}

void CaseRegistry::Save(UCase uc)
{
    ++m_UCases;
    boost::shared_lock lock{ m_Mutex };
    if (uc->Depth != m_MaxDepth)
        throw std::logic_error{ "Depth not matching" };
    push(m_UnsafeCases.front(), uc);
}

void CaseRegistry::Process()
{
    PCase c;

    boost::upgrade_lock lock{ m_Mutex };

again:
    // anything could happen during this time, so check
    if (m_Completed)
        return nullptr;
    if (m_Reaping)
        goto reaping;
    goto forking;

forking:
    if ((c = pop(m_PendingD0Cases))) { return c; }
    // D0 is now empty, we need to enter reaping stage
    if (lock.try_lock_upgrade())
    {
        // we offically enter reaping stage
        m_MaxDepth++, m_Reaping = true;
        m_UnsafeCases.emplace_front();
        m_PendingD0Cases.store(
                m_PendingD1Cases.load(std::memory_order_relaxed),
                std::memory_order_relaxed);
        m_PendingD1Cases.store(nullptr, std::memory_order_relaxed);
        m_CVStage.notify_all();
        goto again;
    }
    goto wait;

reaping:
    while ((c = pop(m_D0SafeCases)))
    {
        ++m_ReapedSCases;
        delete c;
        --m_Borrowed;
    }
    // D0 is now empty, we need to enter forking stage
    if (m_Borrowed.load(std::memory_order_relaxed))
        goto wait;
    {
        boost::upgrade_to_unique_lock wlock{ lock };
        if (m_Borrowed.load(std::memory_order_relaxed))
            goto wait;
        // we offically enter forking stage
        m_Reaping = false;
        m_D0SafeCases.store(
                m_D1SafeCases.load(std::memory_order_relaxed),
                std::memory_order_relaxed);
        m_D1SafeCases.store(nullptr, std::memory_order_relaxed);

        if ((m_Completed = !(c = pop(m_PendingD0Cases))))
            m_CVCompletion.notify_all();
        m_CVStage.notify_all();
        return c;
    }

wait:
    // someone else is still processing, so
    // we are no longer responsible for anything.
    // sleep until the next stage is reached
    m_CVStage.wait(lock);
    goto again;
}

void CaseRegistry::Enqueue(PCase c)
{
    boost::shared_lock lock{ m_Mutex };
    if (m_Completed) throw std::logic_error{ "It shouldn't be completed" };
    if (m_Reaping) throw std::logic_error{ "It shouldn't be reaping" };
    if (c->Depth != m_MaxDepth + 1)
        throw std::logic_error{ "Depth not matching" };
    push(m_PendingD1Cases, c);
}

size_t CaseRegistry::ResolveDanger(HCase root)
{
    foreach(m_ActionCases, [](ACase ac){ ac->ResetDanger(); });
    foreachUnsafeCases([=](UCase uc){ uc->ReportDanger(); });
    root->ResolveDanger();
}

void CaseRegistry::Fork(PCase p)
{
    m_Processed++;
#ifndef NDEBUG
    fmt::print("Queue {}, Root danger = {:8f}%\n", queue.size(), 100.0 * root->Danger / root->TotalStates);
    fmt::print("{1}  (@{0})\n",
            fmt::ptr(p),
            p->ToString());
    std::cin.get();
#endif
    std::vector<PCase> buffer;
    for (PCase pp; (pp = p->CheckedFork());)
    {
#ifndef NDEBUG
        if (auto fc = dynamic_cast<FCase>(p); fc)
            fmt::print("  >>{1}  (@{0}) *{2}\n",
                    fmt::ptr(pp),
                    pp->ToString(),
                    fc->GetDegree() - 1);
        else
            fmt::print("  >>{1}  (@{0})\n",
                    fmt::ptr(pp),
                    pp->ToString());
#endif
        if (p->IsHolder())
            buffer.push_back(pp);
        else
            Enqueue(pp);
    }
    p->Deplete();
    // note: we must fully fork the previous
    // before working on its children
    for (auto pp : buffer)
    {
        m_Processed++;
#ifndef NDEBUG
        fmt::print("  >@{0}\n", fmt::ptr(pp));
#endif
        for (PCase ppp; (ppp = pp->CheckedFork());)
        {
#ifndef NDEBUG
            fmt::print("    >>{1}  (@{0})\n",
                    fmt::ptr(ppp),
                    ppp->ToString());
#endif
            Enqueue(ppp);
        }
        pp->Deplete();
    }
}

template <typename T>
bool write_report(HCase root, T &&t)
{
    std::unique_lock lock{ mtx };
    cve.wait_for(lock, t);

    fmt::print("x");
    auto cnt = g_Registry.ResolveDangerAndReapSafes(root, c.front()->Depth);
    fmt::print("{:.10f}% curr~{:.10f}%@d{}   p{} q{} d{} s{} asu{}+{} t{} m{:.3f}%\n",
            100.0 * root->Danger / root->TotalStates,
            100.0 * c.front()->TotalStates / root->TotalStates,
            c.front()->Depth,
            g_Processed.load(),
            c.size(),
            g_Registry.GetDepth(),
            g_Registry.GetStep(),
            g_Registry.GetCases(),
            cnt,
            g_Trie.size(),
            g_MemoryAvailPercent.load());

    return !done();
}

int main(int argc, char *argv[])
{
    if (argc < 2 || argc > 3)
    {
        std::cout << "Usage: " << argv[0]
            << R"(FL@\[<I>,<J>\]-(NH|2|P|2P)-<W>-<H>-T<M>-(SFAR|SNR) [<nprocs>])"
            << std::endl;
        return 1;
    }

#define CHK(T) \
    fmt::print("sizeof(" #T ")={}\n", sizeof(T))

    CHK(ActionCase);
    CHK(SafeCase);
    CHK(UnsafeCase);

#ifdef NDEBUG
    const bool is_tty = isatty(STDERR_FILENO);
    using namespace std::chrono_literals;
    const auto report_interval = is_tty ? 5s : 60s;
    auto nprocs = argc < 3 ? get_nprocs() : std::atoi(argv[2]);
#else
    auto nprocs = 1;
#endif

    auto cfg = parse(argv[1]);
    if (!cfg.InitialPositionSpecified)
    {
        std::cerr << "You must specify initial position\n";
        return 1;
    }
    cache(cfg);
    g_Strategy = cfg;

    ConcurrentPriorityQueue queue{};
    auto game = std::make_shared<GameMgr>(cfg.Width, cfg.Height, cfg.TotalMines, &g_Strategy);
    auto root = new HolderCase(nullptr, game, 0);
    g_InvalidCase = reinterpret_cast<RCase>(root); // random value
    root->TotalStates = Binomial(cfg.Width * cfg.Height - 1, cfg.TotalMines); // fix the first move
    auto ac = new ActionCase(root, root->ThePGame(), cfg.Index);
    root->AddChildren(ac);
    root->Deplete();
    queue.push(ac);
    queue.inited();

    updateMemoryAvailPercent();
#ifdef NDEBUG
    std::vector<std::thread> threads;
    threads.emplace_back([&]()
    {
        while (queue.sleep_for(2s))
            updateMemoryAvailPercent();
    });
    threads.emplace_back([&]()
    {
        while (queue.write_report(root, report_interval));
    });
#endif

    NanoTimer timer_computation{};
#ifdef NDEBUG
    for (auto i = 0; i < nprocs; i++)
        threads.emplace_back([&]()
        {
#endif
            auto first = true;
            for (PCase p; (p = queue.pop(first)); first = false)
            {
            }
#ifdef NDEBUG
        });
    for (auto &th : threads)
        th.join();
#endif
    g_Registry.ResolveDangerAndReapSafes(root, std::numeric_limits<unsigned>::max());
    timer_computation.stop();

    auto j = to_json(cfg);
    j["string"] = argv[1];
    j["result"]["danger"] = root->Danger;
    j["result"]["ratio"] = 100.0 * root->Danger / root->TotalStates;
    j["exec"]["duration"] = timer_computation.seconds();
    j["exec"]["cpu"] = nprocs;
    j["exec"]["speed"] = static_cast<double>(g_Processed.load()) / timer_computation.seconds() / nprocs;
    std::cout << j << std::endl;
}
