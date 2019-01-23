#include "common.h"
#include <iostream>
#include "game.h"

int main()
{
	game g(30, 16, "FL@[2,2]-NH");

#ifndef NDEBUG
	g.device = std::mt19937_64(114514);
#else
	g.device = std::mt19937_64(std::random_device{}());
#endif
	g.fill_fixed(99, true);

	std::cout << g.grid();
	g.run();
	std::cout << g.grid();

#ifndef NDEBUG
	std::getchar();
#endif
    return 0;
}
