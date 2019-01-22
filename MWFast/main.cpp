#include "common.h"
#include <iostream>
#include "grid.h"

int main()
{
	grid<uint8_t> grid(30, 16, 'a');

	std::cout << grid;

#ifndef NDEBUG
	system("pause");
#endif
    return 0;
}
