#include "facade.hpp"
#include "Prover.h"
#include "GameMgr.h"
#include <algorithm>
#include <condition_variable>
#include <memory>
#include <queue>
#include <mutex>
#include <sstream>

Strategy g_Strategy;

BaseCase::BaseCase(std::unique_ptr<GameMgr> &&game)
    : parent{},
      LogProbability{},
      Duplication{},
      m_Game{ std::move(game) },
      m_SavedGame{} { }

BaseCase::BaseCase(std::shared_ptr<BaseCase> p)
    : parent{ p },
      LogProbability{ p->LogProbability },
      Duplication{},
      m_Game{},
      m_SavedGame{} { }

GameMgr &BaseCase::Game()
{
    if (m_Game)
        return *m_Game;

    std::stringstream ss{ std::move(m_SavedGame) };
    m_Game = std::make_unique<GameMgr>(ss, g_Strategy);
    return *m_Game;
}

BaseCase &BaseCase::Deflate()
{
    if (!m_Game)
        return *this;

    std::stringstream ss;
    m_Game->Save(ss);
    m_SavedGame = ss.str();
    m_Game.reset();
    return *this;
}

class ConcurrentPriorityQueue
{
    std::vector<PCase> c;
    bool finished;
    std::mutex mtx;
    std::condition_variable cv;

    struct Comparer
    {
        bool operator()(const std::shared_ptr<BaseCase> &lhs,
                const std::shared_ptr<BaseCase> &rhs) const
        {
            if (lhs->LogProbability > rhs->LogProbability)
                return true;
            if (lhs->LogProbability < rhs->LogProbability)
                return false;
            return lhs->Duplication < rhs->Duplication;
        }
    };

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

public:
    template <typename T, typename ... Args>
    void emplace(Args && ... args)
    {
        push(std::make_shared<T>(std::forward<Args>(args)...));
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
    auto cfg = parse(argv[1]);
    if (!cfg.InitialPositionSpecified)
    {
        std::cerr << "You must specify initial position\n";
        return 1;
    }
    cache(cfg);
    g_Strategy = cfg;

    ConcurrentPriorityQueue queue{};
    auto root = std::make_shared<BaseCase>(cfg);
    std::make_unique<GameMgr(cfg.Width, cfg.Height, cfg.TotalMines, g_Strategy);

    queue.emplace<ActionCase>(root, cfg.Index);

    PCase p;
    while ((p = queue.pop()))
    {
        auto &g = p->Game();
        if (auto ac = dynamic_cast<ActionCase *>(p.get()); ac)
        {
            auto &d = ac->Dist();
            if (d[0])
                queue.emplace<SafeCase>(ac, std::log(d[0]));
            for (auto i = 1; i <= 8; i++)
                if (d[i])
                    queue.emplace<UnsafeCase>(ac, std::log(d[0]));

            continue;
        }

        if (auto sc = dynamic_cast<SafeCase *>(p.get()); sc)
        {
            auto &g = sc->Game();
            auto id = g.GetBestBlockList().front();
            auto &d = g.GetDistInfo(id);
        }
    }
}
