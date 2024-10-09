#include "BinomialHelper.h"
#include "facade.hpp"
#include "Prover.h"
#include "GameMgr.h"
#include <fmt/ostream.h>
#include <fmt/ranges.h>
#include <condition_variable>
#include <sstream>
#include <stdexcept>
#include <ranges>

Strategy g_Strategy;
static constexpr auto HEUR = SolvingState::Reduce | SolvingState::Overlap | SolvingState::Probability | SolvingState::Heuristic;

BaseCase::BaseCase(PCase p)
    : parent{ p },
      TotalStates{ p->TotalStates },
      Duplication{},
      m_Game{ p->m_Game } { }

BaseCase::BaseCase(PCase p, PGame game)
    : parent{ p },
      TotalStates{ game->GetSolver().GetTotalStates() },
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
    return lhs->Danger < rhs->Danger;
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

#ifndef NDEBUG
        fmt::print("[[[{}@{}+={}->{} in {}@{}]]]\n",
                self->ToString(),
                fmt::ptr(self),
                v,
                self->Danger,
                ToString(),
                fmt::ptr(this));
#endif
        m_Heap.increase(self->Handle);

        auto next = m_Heap.top()->Danger;
        increase = next - Danger;
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
        auto p = Ephermeral ? parent : this;
        if (g->GetBestBlockCount())
            return new SafeCase(p, g);
        return new UnsafeCase(p, g);
    }
    return nullptr;
}

PCase UnsafeCase::Fork()
{
    auto &lst = Game().GetPreferredBlockList();
    while (m_It != lst.end())
    {
        auto ac = new ActionCase(this, ThePGame(), *m_It++);
        AddChildren(ac);
        return ac;
    }

    return nullptr;
}

std::string BaseCase::ToString() const
{
    if (!std::holds_alternative<PGame>(m_Game))
        return fmt::format("[G=0x------ TS={:5e}]",
                TotalStates);
    return fmt::format("[G={} TS={:5e}]",
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
    return fmt::format("{}[{}]",
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
        return fmt::format("Safe{}:S--",
                ForkedCase::ToString());
    return fmt::format("Safe{}:S{}",
            ForkedCase::ToString(),
            std::get<PGame>(m_Game)->GetBestBlockCount());
}

std::string UnsafeCase::ToString() const
{
    return fmt::format("Unsafe{}:D{}",
            HolderCase::ToString(),
            Duplication);
}

class ConcurrentPriorityQueue
{
    std::vector<PCase> c;
    bool finished;
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
            c.push_back(std::move(p));
            auto it = std::ranges::push_heap(c, Comparer{});
            if (std::distance(it, c.begin()) >= 1z << 10)
                (*it)->Deflate();
        }
        cv.notify_one();
    }

    void finish()
    {
        {
            std::lock_guard lock{ mtx };
            finished = true;
        }
        cv.notify_all();
    }

    PCase pop()
    {
        std::unique_lock lock{ mtx };
        cv.wait(lock, [this]{ return finished || !c.empty(); });
        if (finished && c.empty())
            return {};

        std::ranges::pop_heap(c, Comparer{});
        auto p = std::move(c.back());
        c.pop_back();
        return p;
    }
};

int main(int argc, char *argv[]) {
    if (argc != 2)
    {
        std::cerr << "Usage: FL@[<I>,<J>]-(NH|2|P|2P)-<W>-<H>-T<M>-(SFAR|SNR)\n";
        return 1;
    }
    auto cfg = parse(argv[1]);
    if (!cfg.InitialPositionSpecified)
    {
        std::cerr << "You must specify initial position\n";
        return 1;
    }
    cache(cfg);
    g_Strategy = cfg;

    ConcurrentPriorityQueue queue{};
    auto root = new HolderCase(nullptr,
        std::make_shared<GameMgr>(cfg.Width, cfg.Height, cfg.TotalMines, g_Strategy));
    root->TotalStates = Binomial(cfg.Width * cfg.Height - 1, cfg.TotalMines); // fix the first move
    auto ac = new ActionCase(root, root->ThePGame(), cfg.Index);
    root->AddChildren(ac);
    queue.push(ac);

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
        if (dynamic_cast<SafeCase *>(p))
            delete p;
        else
            p->Deplete();
    }
}
