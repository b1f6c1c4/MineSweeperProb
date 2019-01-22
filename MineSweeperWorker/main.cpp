#include "stdafx.h"
#include "Worker.h"
#include "WorkerT.h"
#include <cstring>
#include <iostream>
#include <string>
#include <sstream>
#include <boost/program_options.hpp>
#include "../MineSweeperSolver/ReadStrategy.h"

#define NUMT(X) #X
#define NUM(X) NUMT(X)

namespace po = boost::program_options;

int main(const int argc, char **argv)
{
	po::options_description desc("A deterministic Minesweeper solver");
	desc.add_options()
		("help,h", "produce help message")
		("verbose,v", "show progess")
		("time,t", "enable timing")
		("slack,S", "allow wrong guesses")
		("width,n", po::value<int>(), "grid width")
		("height,m", po::value<int>(), "grid height")
		("fixed-mines,T", po::value<int>(), "total mine count")
		("probability,P", po::value<double>(), "mine probability")
		("strategy,s", po::value<std::string>()->default_value("FL-PSEQ-D256"), "strategy")
		("repetition,N", po::value<size_t>()->default_value(1), "number of evaluations");

	po::variables_map vm;
	store(parse_command_line(argc, argv, desc), vm);
	notify(vm);

	if (vm.count("help") || vm.count("fixed-mines") && vm.count("probability"))
	{
		std::cerr << desc << std::endl;
		return 1;
	}

    if (!vm.count("width") || !vm.count("height"))
    {
        std::cerr << "Invalid grid size" << std::endl;
        return 1;
    }

	const auto width = vm["width"].as<int>();
	const auto height = vm["height"].as<int>();

    if (width <= 1 || height <= 1)
    {
        std::cerr << "Invalid grid size" << std::endl;
        return 1;
    }

    bool is_fixed_mines;
    size_t fixed_mines = 0;
    double probability = 0;
    if (vm.count("fixed-mines"))
    {
        if (vm.count("probability"))
        {
            std::cerr << "Specify either total mine count or mine probability" << std::endl;
            return 1;
        }

        is_fixed_mines = true;
        fixed_mines = vm["fixed-mines"].as<int>();
        if (fixed_mines < 0 || fixed_mines > width * height - 1)
        {
            std::cerr << "Invalid total mine count" << std::endl;
            return 1;
        }
    }
    else if (vm.count("probability"))
    {
        is_fixed_mines = false;
        probability = vm["probability"].as<double>();
        if (probability < 0 || probability > 1)
        {
            std::cerr << "Invalid mine probability" << std::endl;
            return 1;
        }
    }
    else
    {
        std::cerr << "Must specify total mine count or mine probability" << std::endl;
        return 1;
    }

    const bool slack = vm.count("slack");
    const auto strategy = vm["strategy"].as<std::string>();
    const auto rep = vm["repetition"].as<size_t>();

    std::shared_ptr<BaseBaseWorker> bworker;
    std::shared_ptr<AdapterWorker> aworker;
    if (vm.count("timing"))
    {
        auto w = std::make_shared<WorkerT>();
        bworker = w;
        aworker = w;
    }
    else
    {
        auto w = std::make_shared<Worker>();
        bworker = w;
        aworker = w;
    }

    bworker->Repetition = rep;
    aworker->Config.Slack = slack;
    aworker->Config.IsTotalMine = is_fixed_mines;
    aworker->Config.Width = width;
    aworker->Config.Height = height;
    aworker->Config.TotalMines = fixed_mines;
    aworker->Config.Probability = probability;

    if (!ReadStrategy(strategy, aworker->Config, width, height))
    {
        std::cerr << "Invalid strategy" << std::endl;
        return 1;
    }

	if (vm.count("verbose"))
	{
		std::cerr << "n = " << width << std::endl;
		std::cerr << "m = " << height << std::endl;
        if (is_fixed_mines)
            std::cerr << "T = " << fixed_mines << std::endl;
        else
            std::cerr << "P = " << probability << std::endl;
		std::cerr << "Strategy = " << strategy << std::endl;
		std::cerr << "N = " << rep << std::endl;
	}

    bworker->Process();

    for (size_t i = 0; i < bworker->Result.size(); i++)
        std::cout << i << ": " << bworker->Result[i] << std::endl;

    return 0;
}
