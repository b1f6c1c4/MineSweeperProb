#include "GacUI.h"
#include <Windows.h>
#include <vector>
#include "../MineSweeperSolver/BinomialHelper.h"
#include <string>
#include "MineSweeperDemo.h"
#include "boost/program_options.hpp"
#include <iostream>
#include "ReadStrategy.h"

static size_t width, height, totalMines;
static std::shared_ptr<Strategy> strategy;

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int CmdShow)
{
    SetProcessDPIAware();

    auto interval = 1000;
    std::string hsh;


    namespace po = boost::program_options;
    po::options_description desc("Options");
    desc.add_options()
        ("help,h", "Print help messages")
        ("interval,i", po::value<int>(&interval), "Time interval within two moves")
        ("CONFIG", po::value<std::string>(&hsh), "Configuration");

    po::positional_options_description positionalOptions;
    positionalOptions.add("CONFIG", 1);

    po::variables_map vm;
    try
    {
        std::vector<std::string> args;
        args.emplace_back(lpCmdLine);
        po::store(
                  po::command_line_parser(args)
                  .options(desc)
                  .positional(positionalOptions).run(),
                  vm);

        if (vm.count("help"))
        {
            std::cout << "MineSweeperDemo" << std::endl;
            std::cout << desc << std::endl;
            return 0;
        }

        po::notify(vm);

        if (hsh.length() == 0)
            hsh = "FL@[1,1]-PSEQ-D256-30-16-T99-NR";

        strategy = std::make_shared<Strategy>();
        ReadStrategy(hsh, *strategy, width, height, totalMines);
    }
    catch (std::exception &e)
    {
        std::cerr << "ERROR: " << e.what() << std::endl << std::endl;
        std::cerr << desc << std::endl;
        return 1;
    }

    CacheBinomials(width * height, totalMines);

    return SetupWindowsDirect2DRenderer();
}

void GuiMain()
{
    auto window = new MineSweeperDemo(strategy, width, height, totalMines);
    GetApplication()->Run(window);
    delete window;
}
