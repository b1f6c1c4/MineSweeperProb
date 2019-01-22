#include "common.h"
#include <iostream>
#include "game.h"

int main()
{
	game g(30, 16, "PL@[1,1]-NH");

	std::mt19937_64 rnd(std::random_device{}());
	g.fill_fixed(rnd, 99, false);

	std::cout << g.grid();

#ifndef NDEBUG
	system("pause");
#endif
    return 0;
}
