#include "MineSweeperDemo.h"
#include <chrono>
#include <string>

#define UPDATE GetApplication()->InvokeLambdaInMainThread([this]() { this->Update(); })
#define UPDATE_AND_WAIT GetApplication()->InvokeLambdaInMainThreadAndWait([this]() { this->Update(); })

void MineSweeperDemo::MakeTable(size_t width, size_t height)
{
    m_Table = new GuiTableComposition;
    m_Table->SetRowsAndColumns(height, width);
    for (auto i = 0; i < height; ++i)
        m_Table->SetRowOption(i, GuiCellOption::PercentageOption(static_cast<double>(1) / height));
    for (auto i = 0; i < width; ++i)
        m_Table->SetColumnOption(i, GuiCellOption::PercentageOption(static_cast<double>(1) / width));
    m_Table->SetAlignmentToParent(Margin(1, 1, 1, 1));
    m_Table->SetCellPadding(0);
    this->GetContainerComposition()->AddChild(m_Table);

    FontProperties font;
    font.fontFamily = L"Consolas";
    font.size = 30;
    font.antialias = true;

    for (auto j = 0; j < width; ++j)
        for (auto i = 0; i < height; ++i)
            MakeCell(font, i, j);
}

void MineSweeperDemo::MakeCell(const FontProperties &font, int i, int j)
{
    auto cell = new GuiCellComposition;
    m_Table->AddChild(cell);
    cell->SetSite(i, j, 1, 1);

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

MineSweeperDemo::MineSweeperDemo(std::shared_ptr<Strategy> strategy, size_t width, size_t height, size_t totalMines, size_t mult) : GuiWindow(GetCurrentTheme()->CreateWindowStyle()), m_Mult(mult), m_Width(width), m_Height(height), m_TotalMines(totalMines), m_Strategy(strategy), m_Manual(true)
{
    this->GuiControlHost::SetText(L"MineSweeperDemo");
    this->GetContainerComposition()->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);

    MakeTable(m_Width, m_Height);

    this->Update();

    m_Thread = std::thread(&MineSweeperDemo::Process, this);
    m_LastX = 0 , m_LastY = 0;

    this->ForceCalculateSizeImmediately();
    this->SetClientSize(Size(m_Width * 37 + 2, m_Height * 37 + 2));
    this->MoveToScreenCenter();
}

MineSweeperDemo::~MineSweeperDemo() { }

void MineSweeperDemo::Update()
{
    if (m_Mgr == nullptr)
    {
        for (auto i = 0; i < m_Width * m_Height; ++i)
        {
            m_Labels[i]->SetText(L"");
            m_Backs[i]->SetColor(Color(204, 204, 204));
        }
        return;
    }

    for (auto i = 0; i < m_Width * m_Height; ++i)
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

void MineSweeperDemo::KeyUp(const NativeWindowKeyInfo &info)
{
    switch (info.code)
    {
    case 0x20:
        m_Manual ^= true;
        break;
    case 0x52:
        m_Mgr = std::make_unique<GameMgr>(m_Width, m_Height, m_TotalMines, *m_Strategy);
        this->Update();
        break;
    case 0x43:
        this->Update();
        break;
    }
}

bool MineSweeperDemo::CheckMgr()
{
    if (m_Mgr != nullptr)
        return false;

    if (!m_Manual)
    {
        m_Mgr = std::make_unique<GameMgr>(m_Width, m_Height, m_TotalMines, *m_Strategy);
        UPDATE_AND_WAIT;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    return true;
}

bool MineSweeperDemo::ResetMgr()
{
    if (m_Mgr->GetStarted())
        return false;

    UPDATE_AND_WAIT;

    auto s = m_Mgr->GetSucceed();
    m_Mgr.reset();
    std::this_thread::sleep_for(std::chrono::milliseconds(s ? 700 : 1200));
    return true;
}

void MineSweeperDemo::Process()
{
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    while (true)
    {
        if (CheckMgr())
            continue;

        if (ResetMgr())
            continue;

        UPDATE;

        m_Mgr->Solve(SolvingState::Reduce | SolvingState::Overlap, false);
        if (m_Mgr->GetSolver().CanOpenForSure == 0)
            m_Mgr->Solve(SolvingState::Reduce | SolvingState::Overlap | SolvingState::Probability | SolvingState::Heuristic | SolvingState::Drained, true);

        auto flag = false;
        auto cnt = m_Mgr->GetBestBlockCount();
        const Block *ptr;
        if (m_Strategy->InitialPositionSpecified && m_Mgr->GetToOpen() + m_Mgr->GetTotalMines() == m_Mgr->GetTotalWidth() * m_Mgr->GetTotalHeight())
        {
            cnt = 1;
            ptr = &m_Strategy->Index;
        }
        else if (cnt == 0)
        {
            cnt = m_Mgr->GetPreferredBlockCount();
            ptr = m_Mgr->GetPreferredBlocks();
        }
        else
        {
            flag = true;
            ptr = m_Mgr->GetBestBlocks();
        }

        auto bestV = std::numeric_limits<int>::max();
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
            m_LastX = bestBlk / m_Height , m_LastY = bestBlk % m_Height;
            m_Mgr->OpenBlock(m_LastX, m_LastY);
        }

        auto bbb = m_LastX * m_Height + m_LastY;
        GetApplication()->InvokeLambdaInMainThread([this, flag, bbb]()
                                                   {
                                                       this->Update();
                                                       m_Backs[bbb]->SetColor(flag ? Color(255 - 170, 255, 0) : Color(255, 170, 0));
                                                   });
        auto ms = m_Mult; // * static_cast<int>(sqrt(bestV)) + 1;
        if (!flag)
            ms *= 3;
        std::this_thread::sleep_for(std::chrono::milliseconds(ms));
    }
}
