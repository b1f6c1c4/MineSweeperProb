#include "GacUI.h"
#include <Windows.h>
#include <vector>
#include "../MineSweeperSolver/GameMgr.h"
#include "../MineSweeperSolver/BinomialHelper.h"
#include <thread>
#include <string>

std::string StrategyStr = "FL-PSEQZ";

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int CmdShow)
{
    SetProcessDPIAware();
    CacheBinomials(30 * 16, 99);
    return SetupWindowsDirect2DRenderer();
}

class MineSweeperDemo : public GuiWindow
{
public:
    MineSweeperDemo()
        :GuiWindow(GetCurrentTheme()->CreateWindowStyle()),
         m_Manual(true)
    {
        this->GuiControlHost::SetText(L"MineSweeperDemo");
        this->GetContainerComposition()->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);

        auto table = new GuiTableComposition;
        table->SetRowsAndColumns(16, 30);
        for (auto i = 0; i < 16; ++i)
            table->SetRowOption(i, GuiCellOption::PercentageOption(static_cast<double>(1) / 16));
        for (auto i = 0; i < 30; ++i)
            table->SetColumnOption(i, GuiCellOption::PercentageOption(static_cast<double>(1) / 30));
        table->SetAlignmentToParent(Margin(1, 1, 1, 1));
        table->SetCellPadding(0);
        this->GetContainerComposition()->AddChild(table);

        {
            FontProperties font;
            font.fontFamily = L"Consolas";
            font.size = 30;
            font.antialias = true;

            for (auto i = 0; i < 480; ++i)
            {
                auto cell = new GuiCellComposition;
                table->AddChild(cell);
                cell->SetSite(i % 16, i / 16, 1, 1);

                auto label = GuiSolidLabelElement::Create();
                label->SetFont(font);
                label->SetHorizontalAlignment(Alignment::Center);
                label->SetVerticalAlignment(Alignment::Center);
                m_Labels.push_back(label);
                auto labelB = new GuiBoundsComposition;
                labelB->SetAlignmentToParent(Margin(0, 0, 2, 1));
                labelB->SetOwnedElement(label);

                auto color = GuiSolidBackgroundElement::Create();
                m_Backs.push_back(color);
                auto colorB = new GuiBoundsComposition;
                colorB->SetAlignmentToParent(Margin(0, 0, 1, 1));
                colorB->SetOwnedElement(color);

                cell->AddChild(colorB);
                cell->AddChild(labelB);
            }
        }

        this->Update();

        m_Thread = std::thread(&MineSweeperDemo::Process, this);
        m_LastX = 0 , m_LastY = 0;

        this->ForceCalculateSizeImmediately();
        this->SetClientSize(Size(30 * 37 + 2, 16 * 37 + 2));
        this->MoveToScreenCenter();
    }

    ~MineSweeperDemo()
    {
        if (m_Mgr != nullptr)
        {
            delete m_Mgr;
            m_Mgr = nullptr;
        }
    }

    void Update()
    {
        if (m_Mgr == nullptr)
        {
            for (auto i = 0; i < 480; ++i)
            {
                m_Labels[i]->SetText(L"");
                m_Backs[i]->SetColor(Color(204, 204, 204));
            }
            return;
        }

        for (auto i = 0; i < 480; ++i)
        {
            auto &b = m_Mgr->GetBlockProperties()[i];

            auto label = m_Labels[i];
            auto color = m_Backs[i];

            if (b.IsOpen)
            {
                if (b.Degree != 0)
                    label->SetText(itow(b.Degree));
                else if (b.IsMine)
                    label->SetText(L"X");
                else
                    label->SetText(L"");
                if (m_Mgr->GetStarted())
                    color->SetColor(Color(255, 255, 255));
                else if (!b.IsMine)
                    color->SetColor(Color(255, 255, 255));
            }
            else if (!m_Mgr->GetStarted() && b.IsMine)
            {
                if (m_Mgr->GetSolver().GetBlockStatus(i) == BlockStatus::Mine)
                    label->SetText(L"F");
                else
                    label->SetText(L"");
                if (m_Mgr->GetSucceed())
                    color->SetColor(Color(0, 255, 0));
                else
                    color->SetColor(Color(255, 0, 0));
            }
            else
            {
                if (m_Mgr->GetSolver().GetBlockStatus(i) == BlockStatus::Mine)
                    label->SetText(L"F");
                else
                    label->SetText(L"");
                color->SetColor(Color(204, 204, 204));
            }
        }
    }

    void KeyUp(const NativeWindowKeyInfo &info) override
    {
        switch (info.code)
        {
        case 0x20:
            m_Manual ^= true;
            break;
        case 0x52:
            if (m_Mgr != nullptr)
            {
                delete m_Mgr;
                m_Mgr = nullptr;
            }
            m_Mgr = new GameMgr(30, 16, 99);
            m_Mgr->BasicStrategy = ReadStrategy(StrategyStr);
            this->Update();
            break;
        case 0x43:
            this->Update();
            break;
        }
    }

private:
    std::thread m_Thread;
    std::vector<GuiSolidLabelElement *> m_Labels;
    std::vector<GuiSolidBackgroundElement *> m_Backs;
    GameMgr *m_Mgr;
    int m_LastX, m_LastY;
    bool m_Manual;

    void Process()
    {
        std::this_thread::sleep_for(std::chrono::milliseconds{1000});
        while (true)
        {
            if (m_Mgr == nullptr)
            {
                if (!m_Manual)
                {
                    m_Mgr = new GameMgr(30, 16, 99);
                    m_Mgr->BasicStrategy = ReadStrategy(StrategyStr);
                    GetApplication()->InvokeLambdaInMainThreadAndWait([this]()
                                                                      {
                                                                          this->Update();
                                                                      });
                }
                std::this_thread::sleep_for(std::chrono::milliseconds{100});
                continue;
            }

            if (!m_Mgr->GetStarted())
            {
                GetApplication()->InvokeLambdaInMainThreadAndWait([this]()
                                                                  {
                                                                      this->Update();
                                                                  });
                auto s = m_Mgr->GetSucceed();
                delete m_Mgr;
                m_Mgr = nullptr;
                std::this_thread::sleep_for(std::chrono::milliseconds{s ? 700 : 1200});
                continue;
            }

            GetApplication()->InvokeLambdaInMainThread([this]()
                                                       {
                                                           this->Update();
                                                       });

            m_Mgr->Solve(SolvingState::Reduce | SolvingState::Overlap, false);
            if (m_Mgr->GetSolver().CanOpenForSure == 0)
                m_Mgr->Solve(SolvingState::Reduce | SolvingState::Overlap | SolvingState::Probability | SolvingState::Heuristic, true);

            auto flag = false;
            auto cnt = m_Mgr->GetBestBlockCount();
            const Block *ptr;
            if (cnt == 0)
            {
                cnt = m_Mgr->GetPreferredBlockCount();
                ptr = m_Mgr->GetPreferredBlocks();
            }
            else
            {
                flag = true;
                ptr = m_Mgr->GetBestBlocks();
            }

            auto bestV = 30 * 30 + 16 * 16 + 1;
            {
                auto bestBlk = -1;
                for (auto i = 0; i < cnt; ++i)
                {
                    auto blk = ptr[i];
                    auto &b = m_Mgr->GetBlockProperties()[blk];
                    auto v = (b.X - m_LastX) * (b.X - m_LastX) + (b.Y - m_LastY) * (b.Y - m_LastY);
                    if (v < bestV)
                    {
                        bestBlk = blk;
                        bestV = v;
                    }
                }
                m_LastX = bestBlk / 16 , m_LastY = bestBlk % 16;
                m_Mgr->OpenBlock(m_LastX, m_LastY);
            }

            auto bbb = m_LastX * 16 + m_LastY;
            GetApplication()->InvokeLambdaInMainThread([this, flag, bbb]()
                                                       {
                                                           this->Update();
                                                           m_Backs[bbb]->SetColor(flag ? Color(255 - 170, 255, 0) : Color(255, 170, 0));
                                                       });
            auto ms = 5 * static_cast<int>(sqrt(bestV)) + 1;
            if (!flag)
                ms *= 3;
            std::this_thread::sleep_for(std::chrono::milliseconds{ms});
        }
    }
};

void GuiMain()
{
    auto window = new MineSweeperDemo;
    GetApplication()->Run(window);
    delete window;
}
