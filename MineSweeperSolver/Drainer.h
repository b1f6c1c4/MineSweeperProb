#pragma once
#include "BasicDrainer.h"
#include "GameMgr.h"

class GameMgr;

/* Find the location of optimal next move, based on Macro/MicroSituation list.
 */
class
    Drainer : public BasicDrainer
{
public:
    explicit Drainer(const GameMgr &mgr);
    ~Drainer() = default;

    [[nodiscard]] BlockSet GetBestBlocks() const;
    [[nodiscard]] const std::vector<double> &GetBestProbabilityList() const;

    void Update();
protected:
    void HeuristicPruning(MacroSituation *macro, BlockSet &bests) override;

private:
    const GameMgr &m_Mgr;
    std::vector<double> m_Prob;

    BlockSet m_Blocks;
    std::map<Block, Block> m_BlocksLookup;
    std::vector<int> m_DMines;

    int FrontierDist(const MacroSituation *macro, Block blk) const;
};
