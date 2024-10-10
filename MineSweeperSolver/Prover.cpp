#include "BinomialHelper.h"
#include "facade.hpp"
#include "Prover.h"
#include "GameMgr.h"
#include <exception>
#include <fmt/ostream.h>
#include <fmt/ranges.h>
#include <sys/sysinfo.h>
#include <condition_variable>
#include <sstream>
#include <stdexcept>
#include <thread>
#include <ranges>

#include <fstream>

#ifndef TRACEBACK
#define Traceback ""
#endif

Strategy g_Strategy;
static constexpr auto HEUR = SolvingState::Reduce | SolvingState::Overlap | SolvingState::Probability | SolvingState::Heuristic;

std::atomic<unsigned> g_MaxDepth;

auto updateDepth(unsigned d)
{
    auto old = g_MaxDepth.load();
    while (d > old)
        if (g_MaxDepth.compare_exchange_weak(old, d))
            break;
    return d;
}

double getMemoryAvailPercent()
{
    std::ifstream fin("/proc/meminfo");
    size_t total, avail;
    std::string s;
    fin >> s >> total >> s;
    fin >> s >> avail >> s;
    fin >> s >> avail >> s;
    return 100 * avail / total;
}

BaseCase::BaseCase(PCase p)
    : parent{ p },
      TotalStates{ p->TotalStates },
      Depth{ updateDepth(p->Depth + 1) },
      Duplication{},
      m_Game{ p->m_Game } { }

BaseCase::BaseCase(PCase p, PGame game)
    : parent{ p },
      TotalStates{ game->GetSolver().GetTotalStates() },
      Depth{ p ? updateDepth(p->Depth + 1) : 0u },
      Duplication{},
      m_Game{ std::move(game) } { }

BaseCase::~BaseCase() = default;

PGame BaseCase::ThePGame()
{
    if (std::holds_alternative<PGame>(m_Game))
        return std::get<PGame>(m_Game);

    std::stringstream ss{ std::get<std::string>(std::move(m_Game)) };
    m_Game = std::make_shared<GameMgr>(ss, g_Strategy);
    return std::get<PGame>(m_Game);
}

BaseCase &BaseCase::Deflate()
{
    if (std::holds_alternative<std::string>(m_Game))
        return *this;

    std::stringstream ss;
    std::get<PGame>(m_Game)->Save(ss);
    m_Game = ss.str();
    return *this;
}

bool HolderCase::Comparer::operator()(ActionCase *lhs, ActionCase *rhs) const
{
    return lhs->Danger > rhs->Danger;
}

void HolderCase::AddChildren(ActionCase *v)
{
    std::lock_guard lock{ mtx };
    v->Handle = m_Heap.push(v);
}

void HolderCase::ReportDanger(ActionCase *self, double v)
{
    auto increase = 0.0;
    if (!self)
    {
        increase = Danger = v; // initial danger prediction from UnsafeCase
    }
    else
    {
        std::lock_guard lock{ mtx };
        self->Danger += v;

        m_Heap.decrease(self->Handle);

        auto next = m_Heap.top()->Danger;
        increase = next > Danger ? next - Danger : 0;
#ifndef NDEBUG
        if (!increase)
            fmt::print("[[[{}@{}+={:3g}->{:5e} in {}@{}]]]\n",
                    self->ToString(),
                    fmt::ptr(self),
                    v,
                    self->Danger,
                    ToString(),
                    fmt::ptr(this));
        else
            fmt::print("[[[{}@{}+={:3g}->{:5e} in {}@{}:{:5e}+={:5e}->{:5e}]]]\n",
                    self->ToString(),
                    fmt::ptr(self),
                    v,
                    self->Danger,
                    ToString(),
                    fmt::ptr(this),
                    Danger,
                    increase,
                    next);
#endif
        Danger = next;
    }

    if (increase && parent)
    {
#ifndef NDEBUG
        auto ac = dynamic_cast<ActionCase *>(parent);
        if (!ac)
            throw std::logic_error{ "Parent of ActionCase must be HolderCase!" };
#else
        auto ac = reinterpret_cast<ActionCase *>(parent);
#endif
        ac->ReportDanger(increase);
    }
}

void ActionCase::ReportDanger(double v)
{
    if (!v)
        return;
#ifndef NDEBUG
    auto hc = dynamic_cast<HolderCase *>(parent);
    if (!hc)
        throw std::logic_error{ "Parent of ActionCase must be HolderCase!" };
#else
    auto hc = reinterpret_cast<HolderCase *>(parent);
#endif
    hc->ReportDanger(this, v);
}

template <bool Ephermeral>
PCase ForkedCase::Fork()
{
    while (m_Degree <= 8)
    {
        auto g = std::make_shared<GameMgr>(Game());
        g->SetBlockDegree(Id, m_Degree++);
        g->Solve(HEUR, false);
        if (!g->GetStarted()) // infeasible
            continue;
        if (g->GetSolver().GetTotalStates() == 1)
            continue; // guaranteed win
        auto p = Ephermeral ? parent : this;
        auto c = g->GetBestBlockCount()
            ? static_cast<BaseCase *>(new SafeCase(p, g))
            : new UnsafeCase(p, g);
#ifdef TRACEBACK
        c->Traceback = Traceback + fmt::format("{}", m_Degree - 1);
#endif
        return c;
    }
    return nullptr;
}

PCase ActionCase::Fork()
{
    if (!m_Degree)
    {
        auto g = std::make_shared<GameMgr>(Game());
        g->SetBlockMine(Id, false);
        g->Solve(HEUR, false);
        if (!g->GetStarted()) // infeasible at all
        {
            {
                std::ofstream fout("/tmp/what");
                Game().Save(fout);
            }
            fmt::print("Traceback::\n");
            for (BaseCase *ptr = this; ptr; ptr = ptr->parent)
                fmt::print("At {}\n",
                        ptr->ToString());
            fmt::print("=======\n");
            throw std::logic_error{ "All ActionCase should be feasible" };
        }
        if (g->GetSolver().GetTotalStates() == 1)
            return nullptr; // guaranteed win
        m_Game = g;
    }

    return ForkedCase::Fork<false>();
}

PCase UnsafeCase::Fork()
{
    auto &lst = Game().GetPreferredBlockList();
    while (m_It < lst.size())
    {
        auto ac = new ActionCase(this, ThePGame(), lst[m_It++]);
        AddChildren(ac);
        return ac;
    }

    return nullptr;
}

std::string BaseCase::ToString() const
{
    if (!std::holds_alternative<PGame>(m_Game))
        return fmt::format("[dp{} TS={:3g}]",
                Depth,
                TotalStates);
    return fmt::format("[dp{} G={} TS={:3g}]",
            Depth,
            reinterpret_cast<void *>(std::get<PGame>(m_Game).get()),
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
    return fmt::format("{}[{:3g}]",
            BaseCase::ToString(),
            fmt::join(std::views::transform(m_Heap, [](ActionCase *ac){ return ac->Danger; }), " "));
}

std::string ActionCase::ToString() const
{
    return fmt::format("Action{}",
            ForkedCase::ToString());
}

std::string SafeCase::ToString() const
{
    if (!std::holds_alternative<PGame>(m_Game))
        return fmt::format("Safe{}~{}",
                ForkedCase::ToString(),
                Traceback);
    return fmt::format("Safe{}~{}:S{}",
            ForkedCase::ToString(),
            Traceback,
            std::get<PGame>(m_Game)->GetBestBlockCount());
}

std::string UnsafeCase::ToString() const
{
    return fmt::format("Unsafe{}~{}:D{}",
            HolderCase::ToString(),
            Traceback,
            Duplication);
}

class ConcurrentPriorityQueue
{
    std::vector<PCase> c;
    bool initialized;
    size_t borrowed;
    mutable std::mutex mtx;
    std::condition_variable cv;

    struct Comparer
    {
        // check if rhs is more important than lhs
        bool operator()(const PCase &lhs, const PCase &rhs) const
        {
            if (rhs->TotalStates > lhs->TotalStates)
                return true;
            if (rhs->TotalStates < lhs->TotalStates)
                return false;
            return rhs->Duplication > lhs->Duplication;
        }
    };

public:
    [[nodiscard]] auto size() const
    {
        std::lock_guard lock{ mtx };
        return c.size();
    }

    void push(PCase p)
    {
        {
            std::lock_guard lock{ mtx };
            c.push_back(p);
            auto it = std::ranges::push_heap(c, Comparer{});
            if (getMemoryAvailPercent() < 50
                || std::distance(it, c.begin()) >= 1z << 10)
                p->Deflate();
        }
        cv.notify_one();
    }

    // call this when uploaded all seeding tasks
    void inited()
    {
        std::lock_guard lock{ mtx };
        initialized = true;
    }

    // call this after pop()
    void finish()
    {
        auto signal = false;
        {
            std::lock_guard lock{ mtx };
            if (!(--borrowed))
                signal = c.empty();
        }
        // only notify all threads if:
        // 1) no one is generating (borrowed == 0); and
        // 2) no pending task in queue
        if (signal)
            cv.notify_all();
    }

    // remember to call finish() after processing this!
    PCase pop()
    {
        std::unique_lock lock{ mtx };
        // wake me up if there are new tasks or task generation finished
        cv.wait(lock, [this]{ return initialized && !borrowed || !c.empty(); });
        // check if task generation finished
        if (initialized && !borrowed && c.empty())
            return {};

        borrowed++;
        std::ranges::pop_heap(c, Comparer{});
        auto p = std::move(c.back());
        c.pop_back();
        return p;
    }

    bool write_report(HolderCase *root) const
    {
        std::unique_lock lock{ mtx };
        if (initialized && !borrowed && c.empty())
            return false;

        fmt::print("x{:10f}% curr~{:10f}%@d{}   q{} d{} m{:3f}%\n",
                100.0 * root->GetDanger() / root->TotalStates,
                100.0 * c.front()->TotalStates / root->TotalStates,
                c.front()->Depth,
                c.size(),
                g_MaxDepth.load(),
                getMemoryAvailPercent());

        return true;
    }
};

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
    const auto report_interval = is_tty ? 5s : 60s;
    auto nprocs = argc < 3 ? get_nprocs() : std::atoi(argv[2]);
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
    auto game = std::make_shared<GameMgr>(cfg.Width, cfg.Height, cfg.TotalMines, g_Strategy);
    auto root = new HolderCase(nullptr, game);
    root->TotalStates = Binomial(cfg.Width * cfg.Height - 1, cfg.TotalMines); // fix the first move
    auto ac = new ActionCase(root, root->ThePGame(), cfg.Index);
    root->AddChildren(ac);
    root->Deplete();
    queue.push(ac);
    queue.inited();

#ifdef NDEBUG
    std::vector<std::thread> threads;
    threads.emplace_back([&]()
    {
        do
        {
            std::this_thread::sleep_for(report_interval);
        } while (queue.write_report(root));
    });
    for (auto i = 0; i < nprocs; i++)
        threads.emplace_back([&]()
        {
#endif
            PCase p;
            while ((p = queue.pop()))
            {
#ifndef NDEBUG
                fmt::print("Queue {}, Root danger = {:8f}%\n", queue.size(), 100.0 * root->GetDanger() / root->TotalStates);
                fmt::print("{1}  (@{0})\n",
                        reinterpret_cast<void *>(p),
                        p->ToString());
                std::cin.get();
#endif

                for (PCase pp; (pp = p->Fork());)
                {
#ifndef NDEBUG
                    if (auto fc = dynamic_cast<ForkedCase *>(p); fc)
                        fmt::print("  >>{1}  (@{0}) *{2}\n",
                                reinterpret_cast<void *>(pp),
                                pp->ToString(),
                                fc->GetDegree() - 1);
                    else
                        fmt::print("  >>{1}  (@{0})\n",
                                reinterpret_cast<void *>(pp),
                                pp->ToString());
#endif
                    queue.push(pp);
                }
                p->Deplete();

                queue.finish();
            }
#ifdef NDEBUG
        });
    for (auto &th : threads)
        th.join();
#endif

    std::cout << root->GetDanger();
}
