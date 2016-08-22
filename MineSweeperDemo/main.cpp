#include "GacUI.h"
#include <Windows.h>
#include <vector>
#include "../MineSweeperSolver/BinomialHelper.h"
#include "MineSweeperDemo.h"
#include "ReadStrategy.h"

static int interval = 50;
static size_t width, height, totalMines;
static std::shared_ptr<Strategy> strategy;

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int CmdShow)
{
    SetProcessDPIAware();

    std::string hsh;
    if (strlen(lpCmdLine) == 0)
        hsh = "FL@[1,1]-PSEQ-D128-30-16-T99-NR";
    else
        hsh = lpCmdLine;

    strategy = std::make_shared<Strategy>();
    ReadStrategy(hsh, *strategy, width, height, totalMines);

    CacheBinomials(width * height, totalMines);

    return SetupWindowsDirect2DRenderer();
}

void GuiMain()
{
    auto window = new MineSweeperDemo(strategy, width, height, totalMines, interval);
    GetApplication()->Run(window);
    delete window;
}
