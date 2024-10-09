#include "facade.hpp"
#include "Prover.h"
#include "GameMgr.h"
#include <fmt/ranges.h>
#include <algorithm>
#include <condition_variable>
#include <memory>
#include <queue>
#include <mutex>
#include <sstream>

Strategy g_Strategy;

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

PCase ForkedCase::Fork()
{
    while (m_Degree <= 8)
    {
        auto g = std::make_shared<GameMgr>(Game());
        g->SetBlockDegree(Id, m_Degree++);
        g->Solve(SolvingState::Heuristic, false);
        auto ts = g->GetSolver().GetTotalStates();
        if (!ts)
            continue;
        if (g->GetBestBlockCount())
            return new SafeCase(this, g);
        return new UnsafeCase(this, g);
    }
    return nullptr;
}

PCase UnsafeCase::Fork()
{
    auto &lst = Game().GetPreferredBlockList();
    while (m_It != lst.end())
    {
        return new ActionCase(this, ThePGame(), *m_It);
    }

    return nullptr;
}

class ConcurrentPriorityQueue
{
    std::vector<PCase> c;
    bool finished;
    std::mutex mtx;
    std::condition_variable cv;

    struct Comparer
    {
        bool operator()(const PCase &lhs,
                const PCase &rhs) const
        {
            if (lhs->TotalStates > rhs->TotalStates)
                return true;
            if (lhs->TotalStates < rhs->TotalStates)
                return false;
            return lhs->Duplication < rhs->Duplication;
        }
    };

public:
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

    template <typename T, typename ... Args>
    void emplace(Args && ... args)
    {
        push(new T(std::forward<Args>(args)...));
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
    auto root = new BaseCase(nullptr,
        std::make_shared<GameMgr>(cfg.Width, cfg.Height, cfg.TotalMines, g_Strategy));

    queue.emplace<ActionCase>(root, root->ThePGame(), cfg.Index);

    PCase p;
    while ((p = queue.pop()))
    {
        for (PCase pp; (pp = p->Fork());)
            queue.push(pp);
    }
}
