#include <iostream>

#include "facade.hpp"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cout << "Usage: " << argv[0] << R"( [PSDF]L(@\[<I>,<J>\])?-(NH|Pure|[PZSEQFU]+)(-D<D>)?-<W>-<H>-T<M>-(SFAR|SNR))" << std::endl;
        return 1;
    }

    auto cfg = parse(argv[1]);
    cache(cfg);
    auto res = run(cfg);
    if (res)
        std::cout << "S" << std::endl;
    else
        std::cout << "F" << std::endl;

    return 0;
}
