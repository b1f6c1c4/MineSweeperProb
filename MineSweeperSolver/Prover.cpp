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

Strategy g_Strategy;
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

std::vector<__uint128_t> HashCache::cache{ 1u };
void HashCache::ensure(size_t sz) {
    while (cache.size() < sz)
        cache.push_back(cache.back() * Base % Modulo);
}
void HashCache::set(__uint128_t &v, int id, unsigned n) {
    v = (cache[id] * n + v) % Modulo;
}

BaseCase::BaseCase(PCase p)
    : TotalStates{ p->TotalStates },
      Depth{ p->Depth },
      Step{ p->Step },
      Hash{ p->Hash },
      RegistryNext{},
      m_Game{ p->m_Game } { }

BaseCase::BaseCase(PCase p, PGame game)
    : TotalStates{ game->GetSolver().GetTotalStates() },
      Depth{ p ? p->Depth : 0u },
      Step{ p ? p->Step : 0u },
      Hash{ p ? p->Hash : 0ull },
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

        auto next_hash = Hash;
        HashCache::set(next_hash, Id, m_Degree);

        auto c = g_Trie.Find(next_hash);
        if (!c)
        {
            auto g = std::make_shared<GameMgr>(Game());
            g->SetBlockDegree(Id, m_Degree);
            g->Solve(HEUR, false);
            if (!g->GetStarted() // infeasible
                    || g->GetSolver().GetTotalStates() == 1) // guaranteed win
            {
                g_Trie.Emplace(new IgnorableCase(next_hash));
                continue;
            }

            if (g->GetBestBlockCount())
                c = new SafeCase(this, g);
            else
                c = new UnsafeCase(this, g);
            c->operator PCase()->Hash = next_hash;
            c->AssignParent(this);
#ifdef TRACEBACK
            c->operator PCase()->Traceback = Traceback + fmt::format("[{}]={}", Id, m_Degree);
#endif
            g_Trie.Emplace(c);
            m_Degree++;
            return c;
        }
        if (c->TotalStates() == 0)
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

SafeCase::SafeCase(PCase p, PGame g)
    : ForkedCase{ p, g, g->GetBestBlockList().front() }
{
    ++Depth;
}

UnsafeCase::UnsafeCase(PCase p, PGame g)
    : HolderCase{ p, g, g->GetMinProbability() * BaseCase::TotalStates },
      m_List{ std::move(const_cast<BlockSet &>(Game().GetPreferredBlockList())) },
      m_It{ m_List.begin() }
{
    ++Depth;
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
        return fmt::format("[d{}s{} TS{}]",
                Depth,
                Step,
                TotalStates);
    return fmt::format("[d{}s{} G={} TS{}]",
            Depth,
            Step,
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

    // if returning false, wlock is NOT granted
    auto Upgrade() { return Upgrader{ *this }; }
};

template <typename T>
    requires std::derived_from<T, BaseCase>
bool Tier::Push(T *c, ThreadLocalList<T> &lst)
{
    StupidLock lock{ m_Mutex };
    for (auto ptr = lst.front; ptr; ptr = ptr->RegistryNext)
        if (*c == *ptr) // TODO
            return false;
    auto wlock = lock.Upgrade();
    for (auto ptr = lst.front; ptr; ptr = ptr->RegistryNext)
        if (*c == *ptr) // TODO
            return false;
    c->RegistryNext = lst.front;
    lst.front = c;
    m_Size++;
    return true;
}

void CaseRegistry::updateMax(std::atomic<unsigned> &v, unsigned d)
{
    auto old = v.load();
    while (d > old)
        if (v.compare_exchange_weak(old, d))
            break;
    return;
}

bool CaseRegistry::Enqueue(RCase rc)
{
    auto pc = rc->operator PCase();
    auto ts = pc->TotalStates;
    auto cutoff = m_D1Cutoff.load(std::memory_order_relaxed);
    if (ts < cutoff)
    {
        delete rc;
        return;
    }
    auto &tier = [this,ts] -> auto & {
        StupidLock lock{ m_D1Mutex };
        if (!m_D1Quota)
        {
            auto wlock = lock.Upgrade();
            if (!m_D1Quota)
        }
        if (auto it = m_D1Map.find(ts); it != m_D1Map.end())
            return it->second;
        auto wlock = lock.Upgrade();
        return m_D1Map.try_emplace(ts).second;
    }();
    bool flag;
    if (pc->IsHolder())
    {
        flag = tier << static_cast<UCase>(pc);
        ++m_UCases;
        m_UMem += pc->ThePGame()->MemoryFootprint();
        pc->Deflate();
    }
    else
    {
        flag = tier << static_cast<SCase>(pc);
        ++m_SCases;
        m_SMem += pc->ThePGame()->MemoryFootprint();
    }
        pc->Deflate();
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
    m_SMem -= sc->ThePGame()->MemoryFootprint();
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
    m_AMem += ac->ThePGame()->MemoryFootprint();
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
    auto acnt = m_ACases.load(std::memory_order_relaxed);
    auto scnt = m_SCases.load(std::memory_order_relaxed);
    auto ucnt = m_UCases.load(std::memory_order_relaxed);
    auto amem = m_AMem.load(std::memory_order_relaxed);
    auto smem = m_SMem.load(std::memory_order_relaxed);
    auto umem = m_UMem.load(std::memory_order_relaxed);
    fmt::print("{:.10f}% d{} s{} d0={:.2e} d1={:.2e} a{:.2f}GiB s{:.2f}GiB u{:.2f}GiB a{:.1f}B s{:.1f}B u{:.1f}B t{:.2f}GiB m{:.3f}%\n",
            100.0 * root->Danger / root->TotalStates,
            m_MaxDepth,
            m_MaxStep.load(std::memory_order_relaxed),
            m_D0.load(std::memory_order_relaxed) + 0.0,
            m_D1.load(std::memory_order_relaxed) + 0.0,
            amem * GiB,
            smem * GiB,
            umem * GiB,
            static_cast<double>(amem) / acnt,
            static_cast<double>(smem) / scnt,
            static_cast<double>(umem) / ucnt,
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
