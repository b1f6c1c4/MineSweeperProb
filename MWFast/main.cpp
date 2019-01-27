#include "common.h"
#include <iostream>
#include <boost/program_options.hpp>
#include "game.h"

namespace po = boost::program_options;

template <class T = std::mt19937, size_t N = T::state_size>
auto properly_seeded_random_engine() -> typename std::enable_if<!!N, T>::type
{
	typename T::result_type random_data[N];
	std::random_device source;
	std::generate(std::begin(random_data), std::end(random_data), std::ref(source));
	std::seed_seq seeds(std::begin(random_data), std::end(random_data));
	T seeded_engine(seeds);
	return seeded_engine;
}

int main(const int argc, char **argv)
{
	try
	{
		po::options_description desc("A deterministic Minesweeper solver");
		desc.add_options()
			("help,h", "produce help message")
			("verbose,v", "show progess")
			("buff,b", "first step must be zero")
			("width,n", po::value<int>(), "grid width")
			("height,m", po::value<int>(), "grid height")
			("fixed-mines,T", po::value<int>(), "total mine count")
			("probability,P", po::value<double>(), "mine probability")
			("strategy,s", po::value<std::string>()->default_value("FL@[1,1]-PSEQ"), "strategy")
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
		const bool buff = vm.count("buff");

		if (width <= 0 || height <= 0)
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

		const auto strategy = vm["strategy"].as<std::string>();
		const auto rep = vm["repetition"].as<size_t>();

		if (vm.count("verbose"))
		{
			std::cerr << "Grid = " << width << "-";
			std::cerr << height << "-";
			if (is_fixed_mines)
				std::cerr << "T" << fixed_mines;
			else
				std::cerr << "P" << probability;
			if (buff)
				std::cerr << "B";
			std::cerr << std::endl;

			std::cerr << "Strategy = " << strategy << std::endl;
			std::cerr << "N = " << rep << std::endl;
		}
		size_t success_cnt = 0;
		for (size_t i = 0; i < rep; i++)
		{
			game g(width, height, strategy);
#ifndef NDEBUG
			g.device = std::mt19937_64(114514 + i + 22);
#else
			g.device = properly_seeded_random_engine<std::mt19937_64>();
#endif

			if (is_fixed_mines)
				g.fill_fixed(fixed_mines, buff);
			else
				g.fill_prob(probability, buff);

			const auto res = g.run();
#ifndef NDEBUG
			std::cerr << g.grid();
			std::cerr << (res ? "true" : "false") << std::endl;
			std::cerr << g.grid();
#endif
			if (res)
				success_cnt++;
		}

		std::cout << "+" << success_cnt << "," << rep - success_cnt << std::endl;
	}
	catch (std::exception &e)
	{
		std::cerr << "Exception: " << e.what() << std::endl;
		return 2;
	}

	return 0;
}
