#include "BinomialHelper.h"
#include "facade.hpp"
#include "Prover.h"
#include "GameMgr.h"
#include "Util.h"
#include <atomic>
#include <boost/chrono/duration.hpp>
#include <mimalloc-new-delete.h>
#include <fmt/ostream.h>
#include <fmt/ranges.h>
#include <iterator>
#include <sys/sysinfo.h>
#include <sstream>
#include <stdexcept>
#include <thread>
#include <ranges>

#ifndef TRACEBACK
#define Traceback ""
#endif

static constexpr auto GiB = 1.0 / 1024 / 1024 / 1024;

Trie g_Trie{};
Strategy g_Strategy;
RCase g_InvalidCase;
static constexpr auto HEUR = SolvingState::Reduce | SolvingState::Overlap | SolvingState::Probability | SolvingState::Heuristic;

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

void Trie::Dispose()
{
    [](this auto &&self, node_t *n) -> void {
        for (auto &atm : n->next)
            if (auto v = atm.load(std::memory_order_acquire); v)
                self(v), delete v;
    }(&root);
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
      LargestModifiedIndex{ p->LargestModifiedIndex },
      RegistryNext{},
      m_Game{ p->m_Game } { }

BaseCase::BaseCase(PCase p, PGame game)
    : TotalStates{ game->GetSolver().GetTotalStates() },
      Depth{ p ? p->Depth : 0u },
      Step{ p ? p->Step : 0u },
      LargestModifiedIndex{ p ? p->LargestModifiedIndex : 0 },
      RegistryNext{},
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

void BaseCase::Deflate()
{
    if (!std::holds_alternative<PGame>(m_Game))
        return;

    std::stringstream ss;
    std::get<PGame>(m_Game)->Save(ss);
    m_Game = ss.str();
    return;
}

void HolderCase::AddChildren(ACase v)
{
    v->Sibling = Child;
    Child = v;
}

void ReportingCase::AssignParent(FCase p)
{
    std::lock_guard lock{ ParentsMtx };
    if (p->IsAction())
        AllParents.insert(static_cast<ACase>(p));
    else
#ifdef __cpp_lib_containers_ranges
        AllParents.insert_range(static_cast<SCase>(p)->AllParents);
#else
        std::ranges::copy(static_cast<SCase>(p)->AllParents, std::inserter(AllParents, AllParents.end()));
#endif
}

const BaseCase *ReportingCase::GetAnyParent() const
{
    std::lock_guard lock{ ParentsMtx };
    if (AllParents.empty())
        return nullptr;
    return *AllParents.begin();
}

RCase ForkedCase::Fork()
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
            return c;
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
    ++Step;
}

RCase ActionCase::Fork()
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
    ++Depth;
    LargestModifiedIndex = std::max(LargestModifiedIndex, lmi);
}

UnsafeCase::UnsafeCase(PCase p, PGame g, int lmi)
    : HolderCase{ p, g, g->GetMinProbability() * TotalStates },
      m_List{ std::move(const_cast<BlockSet &>(Game().GetPreferredBlockList())) },
      m_It{ m_List.begin() }
{
    ++Depth;
    LargestModifiedIndex = lmi;
}

void UnsafeCase::ResolveParents()
{
    m_CachedParents.assign(AllParents.begin(), AllParents.end());
    AllParents.clear();
}

ACase UnsafeCase::Fork()
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
        m_CachedParents | std::views::transform([](FCase c) { return fmt::ptr(c); }));
#endif
    for (auto ac : m_CachedParents)
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
                AllParents | std::views::transform([](FCase c) { return fmt::ptr(c); }));
    return fmt::format("Safe{}~{}:S{}P{}",
            ForkedCase::ToString(),
            Traceback,
            std::get<PGame>(m_Game)->GetBestBlockCount(),
            AllParents | std::views::transform([](FCase c) { return fmt::ptr(c); }));
}

std::string UnsafeCase::ToString() const
{
    return fmt::format("Unsafe{}~{}:P{}+{}",
            HolderCase::ToString(),
            Traceback,
            AllParents.size(),
            m_CachedParents.size());
}

void CaseRegistry::updateMax(std::atomic<unsigned> &v, unsigned d)
{
    auto old = v.load();
    while (d > old)
        if (v.compare_exchange_weak(old, d))
            break;
    return;
}

void CaseRegistry::Enqueue(RCase rc, TLL &rcs)
{
    auto pc = rc->operator PCase();
    if (pc->IsHolder())
    {
        rcs.ucs << static_cast<UCase>(pc);
        ++m_UCases;
    }
    else
    {
        rcs.scs << static_cast<SCase>(pc);
        ++m_SCases;
    }
}

void CaseRegistry::Process(SCase sc, TLL &rcs)
#ifdef TRACEBACK
try
#endif
{
    --m_D0;
#ifndef NDEBUG
    fmt::print("{1}  (@{0})\n", fmt::ptr(sc), sc->ToString());
    std::cin.get();
#endif
    for (RCase rc; (rc = sc->Fork());)
    {
#ifndef NDEBUG
        fmt::print("  >>{1}  (@{0}) *{2}\n",
                fmt::ptr(rc),
                rc->operator PCase()->ToString(),
                sc->GetDegree() - 1);
#endif
        ++m_D1;
#ifndef NDEBUG
        if (rc->operator PCase()->Depth != m_MaxDepth)
            throw std::logic_error{ "Depth not matching" };
#endif
        Enqueue(rc, rcs);
    }
    // no need to sc->Deplete, it will be deleted
    --m_SCases;
}
#ifdef TRACEBACK
catch (const std::exception &err)
{
    fmt::print("Error: {}\n",
            err.what());
    sc->PrintTraceback();
    throw;
}
#endif

void CaseRegistry::Process(UCase uc, TLL &rcs)
#ifdef TRACEBACK
try
#endif
{
    --m_D0;
    uc->ResolveParents();
#ifndef NDEBUG
    fmt::print("{1}  (@{0})\n", fmt::ptr(uc), uc->ToString());
    std::cin.get();
#endif
    updateMax(m_MaxStep, uc->Step + 1);
    ThreadLocalList<ActionCase> tll;
    for (ACase ac; (ac = uc->Fork());)
    {
#ifndef NDEBUG
        if (ac->Depth != m_MaxDepth - 1)
            throw std::logic_error{ "Depth not matching" };
#endif
        ++m_ACases;
        tll << ac;
        Process(ac, rcs);
    }
    std::move(tll) >> m_ActionCases;
    uc->Deplete();
}
#ifdef TRACEBACK
catch (const std::exception &err)
{
    fmt::print("Error: {}\n",
            err.what());
    uc->PrintTraceback();
    throw;
}
#endif

void CaseRegistry::Process(ACase ac, TLL &rcs)
#ifdef TRACEBACK
try
#endif
{
#ifndef NDEBUG
    fmt::print("  >>{1}  (@{0})\n", fmt::ptr(ac), ac->ToString());
#endif
    for (RCase rc; (rc = ac->Fork());)
    {
#ifndef NDEBUG
        fmt::print("    >>{1}  (@{0})\n",
                fmt::ptr(rc),
                rc->operator PCase()->ToString());
#endif
        ++m_D1;
#ifndef NDEBUG
        if (rc->operator PCase()->Depth != m_MaxDepth)
            throw std::logic_error{ "Depth not matching" };
#endif
        Enqueue(rc, rcs);
    }
    ac->Deplete();
}
#ifdef TRACEBACK
catch (const std::exception &err)
{
    fmt::print("Error: {}\n",
            err.what());
    ac->PrintTraceback();
    throw;
}
#endif

template <typename M>
struct StupidLock
{
    M &mtx;
    bool is_locked;

    explicit StupidLock(M &m)
        : mtx{ m }, is_locked{} { lock(); }

    ~StupidLock() { if (is_locked) unlock(); }

    void unlock()
    {
        if (!is_locked)
            throw std::logic_error{ "Unlocking twice" };
        mtx.unlock_shared();
        is_locked = false;
    }

    void lock()
    {
        if (is_locked)
            throw std::logic_error{ "Unlocking twice" };
        mtx.lock_shared();
        is_locked = true;
    }

    struct Upgrader
    {
        StupidLock &lck;
        bool is_locked;
        explicit Upgrader(StupidLock &l)
            : lck{ l }, is_locked{ lck.mtx.try_unlock_shared_and_lock() } { }
        ~Upgrader()
        {
            if (is_locked)
                lck.mtx.unlock_and_lock_shared();
        }
        operator bool() { return is_locked; }
    };

    auto Upgrade() { return Upgrader{ *this }; }
};

// you must hold wlock of m_Mutex before calling this!
template <typename T>
void operator>>(std::atomic<T> &p, std::atomic<T> &q)
{
    q.store(p.exchange({}, std::memory_order_acquire), std::memory_order_release);
}

void CaseRegistry::Process()
{
    SCase sc;
    UCase uc;
    TLL rcs;
    TLLU ucx;

    StupidLock lock{ m_Mutex };
again:
    // anything could happen during this time, so check
    if (m_Completed)
        return;
lagain:
    while ((sc = pop(m_D0SafeCases)))
        Process(sc, rcs), delete sc;
    while ((uc = pop(m_D0UnsafeCases)))
        Process(uc, rcs), ucx << uc;
    std::move(rcs.scs) >> m_D1SafeCases;
    std::move(rcs.ucs) >> m_D1UnsafeCases;
    std::move(ucx) >> m_UnsafeCases.front();
    // D0 is now empty, we need to enter next stage
    if (auto wlock = lock.Upgrade(); !wlock)
    {
        // someone else is still processing, so
        // we are no longer responsible for anything.
        // sleep until the next stage is reached.
        // don't sleep indefinitely though, as some threads
        // may shared_lock{ m_Mutex } without doing stage change
        m_CVStage.wait_for(lock, boost::chrono::milliseconds{ 10 });
        goto again;
    }
    else // necessary `else' here to keep wlock alive
    {
        // we can offically enter next stage
        m_MaxDepth++;
        m_UnsafeCases.emplace_front();
        m_D1 >> m_D0;
        m_D1SafeCases >> m_D0SafeCases;
        m_D1UnsafeCases >> m_D0UnsafeCases;
        if (!m_D0SafeCases.load(std::memory_order_relaxed)
            && !m_D0UnsafeCases.load(std::memory_order_relaxed))
        {
            m_Completed = true;
            m_CVStage.notify_all();
            m_CVCompletion.notify_all();
            return;
        }
        m_CVStage.notify_all();
        goto lagain;
    }
}

void CaseRegistry::ResolveDanger()
{
    boost::shared_lock lock{ m_Mutex };
    ResolveDangerImpl();
}

void CaseRegistry::ResolveDangerImpl()
{
    foreach(m_ActionCases, [](ACase ac){ ac->ResetDanger(); });
    foreachUnsafeCases([](UCase uc){ uc->ReportDanger(); });
    root->ResolveDanger();
}

CaseRegistry::CaseRegistry(HCase root, int id)
    : m_MaxDepth{ 1 }, m_Completed{}, root{ root }
{
    auto ac = new ActionCase(root, root->ThePGame(), id);
    root->AddChildren(ac);
    root->Deplete();
    ThreadLocalList<ActionCase>{ ac } >> m_ActionCases;

    TLL rcs;
    Process(ac, rcs);
    m_UnsafeCases.emplace_front();
    std::move(rcs.scs) >> m_D1SafeCases;
    std::move(rcs.ucs) >> m_D1UnsafeCases;
}

void CaseRegistry::Dispose()
{
    boost::unique_lock lock{ m_Mutex };
    foreach(m_ActionCases, [](ACase ac){ delete ac; });
    foreachUnsafeCases([](UCase uc){ delete uc; });
}

void CaseRegistry::WriteReport()
{
    ResolveDangerImpl();
    fmt::print("{:.10f}% d{} s{} d0={:.2e} d1={:.2e} a{:.2f}GiB s{:.2f}GiB u{:.2f}GiB t{:.2f}GiB m{:.3f}%\n",
            100.0 * root->Danger / root->TotalStates,
            m_MaxDepth,
            m_MaxStep.load(std::memory_order_relaxed),
            m_D0.load(std::memory_order_relaxed) + 0.0,
            m_D1.load(std::memory_order_relaxed) + 0.0,
            m_ACases.load(std::memory_order_relaxed) * sizeof(ActionCase) * GiB,
            m_SCases.load(std::memory_order_relaxed) * sizeof(SafeCase) * GiB,
            m_UCases.load(std::memory_order_relaxed) * sizeof(UnsafeCase) * GiB,
            g_Trie.size() * GiB,
            g_MemoryAvailPercent.load(std::memory_order_relaxed));
}

template <class Rep, class Period>
auto chronoAdapter(std::chrono::duration<Rep, Period> dur)
{
    return boost::chrono::duration<Rep, Period>{ dur.count() };
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

#ifdef NDEBUG
    const bool is_tty = isatty(STDERR_FILENO);
    using namespace std::chrono_literals;
    const auto report_interval = chronoAdapter(is_tty ? 5s : 60s);
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

    auto game = std::make_shared<GameMgr>(cfg.Width, cfg.Height, cfg.TotalMines, &g_Strategy);
    auto root = new HolderCase(nullptr, game, 0);
    g_InvalidCase = reinterpret_cast<RCase>(root); // random value
    root->TotalStates = Binomial(cfg.Width * cfg.Height - 1, cfg.TotalMines); // fix the first move

    CaseRegistry cr{ root, cfg.Index };

    updateMemoryAvailPercent();
#ifdef NDEBUG
    std::vector<std::thread> threads;
    threads.emplace_back([&]()
    {
        while (cr.Wait(chronoAdapter(2s)))
            updateMemoryAvailPercent();
    });
    threads.emplace_back([&]()
    {
        while (cr.WriteReport(report_interval));
    });
#endif

    NanoTimer timer_computation{};
#ifdef NDEBUG
    for (auto i = 0; i < nprocs; i++)
        threads.emplace_back([&]()
        {
#endif
            cr.Process();
#ifdef NDEBUG
        });
    for (auto &th : threads)
        th.join();
#endif
    cr.ResolveDanger();
    timer_computation.stop();

    auto j = to_json(cfg);
    j["string"] = argv[1];
    j["result"]["danger"] = root->Danger;
    j["result"]["ratio"] = 100.0 * root->Danger / root->TotalStates;
    j["exec"]["duration"] = timer_computation.seconds();
    j["exec"]["cpu"] = nprocs;
    std::cout << j << std::endl;

    cr.Dispose();
    g_Trie.Dispose();
    delete root;
}
