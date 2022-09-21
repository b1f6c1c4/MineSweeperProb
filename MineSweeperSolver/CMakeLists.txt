cmake_minimum_required(VERSION 3.21)
project(MineSweeperSolver)

set(CMAKE_CXX_STANDARD 17)

add_compile_options(-Wall -Wextra -Werror)
add_compile_options(-Wno-parentheses)
add_compile_options(-Wno-missing-field-initializers)
add_compile_options(-Wno-unused-function)
add_compile_options(-Wno-unused-label)
add_compile_options(-Wno-unused-parameter)
add_compile_options(-Wno-unknown-pragmas)
add_compile_options(-Wno-sign-compare)
set(CMAKE_SHARED_LINKER_FLAGS "-Wl,--no-undefined")
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)

find_package(Boost 1.80.0 REQUIRED)

add_executable(MineSweeperSolver
        BasicDrainer.cpp
        BasicSolver.cpp
        BinomialHelper.cpp
        Drainer.cpp
        GameMgr.cpp
        random.cpp
        Solver.cpp
        facade.cpp
        main.cpp)
target_link_libraries(MineSweeperSolver PRIVATE boost_thread)
target_link_libraries(MineSweeperSolver PRIVATE pthread)
