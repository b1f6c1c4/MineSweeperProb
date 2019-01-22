#include "common.h"
#include <iostream>
#include "grid.h"
#include "block.h"

int main()
{
	grid<blk_t> grid(30, 16, 5);

	std::cout << grid;

#ifndef NDEBUG
	system("pause");
#endif
    return 0;
}
