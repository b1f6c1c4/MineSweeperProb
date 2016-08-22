#pragma once
#include "GacUI.h"
#include <vector>
#include "../MineSweeperSolver/GameMgr.h"
#include <thread>

class MineSweeperDemo : public GuiWindow
{
public:
    MineSweeperDemo(std::shared_ptr<Strategy> strategy, size_t width, size_t height, size_t totalMines);

    ~MineSweeperDemo();

    void Update();

    void KeyUp(const NativeWindowKeyInfo &info) override;

private:
    size_t m_Width, m_Height, m_TotalMines;
    std::shared_ptr<Strategy> m_Strategy;

    std::thread m_Thread;
    std::vector<GuiSolidLabelElement *> m_Labels;
    std::vector<GuiSolidBackgroundElement *> m_Backs;
    std::unique_ptr<GameMgr> m_Mgr;
    int m_LastX, m_LastY;
    bool m_Manual;

    GuiTableComposition *m_Table;

    void MakeTable(size_t width, size_t height);
    void MakeCell(const FontProperties &font, int i, int j);

    void Process();
};
