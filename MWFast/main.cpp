#include "common.h"
#include <iostream>
#include "game.h"

int main()
{
	game g(30, 16);
	std::mt19937_64 rnd(std::random_device{}());
	g.fill_fixed(rnd, 99);

	std::cout << g.grid();

#ifndef NDEBUG
	system("pause");
#endif
    return 0;
}
