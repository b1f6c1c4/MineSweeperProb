#include "common.h"
#include <iostream>
#include "game.h"

int main()
{
	game g(6, 1, "FL@[2,1]-PSEQZ");
	//game g(5, 5, "FL@[1,1]-PSEQZ");

#ifndef NDEBUG
	g.device = std::mt19937_64(114514);
#else
	g.device = std::mt19937_64(std::random_device{}());
#endif
	g.fill_fixed(2, false);

	std::cout << g.grid();
	g.run();
	std::cout << g.grid();

#ifndef NDEBUG
	std::getchar();
#endif
    return 0;
}
