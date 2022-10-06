cmake_minimum_required(VERSION 3.21)
project(MineSweeperSolver)

set(CMAKE_CXX_STANDARD 23)

add_compile_options(-fuse-ld=lld)
add_compile_options(-Wall -Wextra -Werror)
add_compile_options(-Wno-parentheses)
add_compile_options(-Wno-missing-field-initializers)
add_compile_options(-Wno-unused-function)
add_compile_options(-Wno-unused-label)
add_compile_options(-Wno-unused-parameter)
add_compile_options(-Wno-unknown-pragmas)
add_compile_options(-Wno-sign-compare)
add_compile_options(-Wno-unused-command-line-argument)
set(CMAKE_SHARED_LINKER_FLAGS "-Wl,--no-undefined")
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)

include(FetchContent)

set(JSON_MultipleHeaders OFF)
FetchContent_Declare(json
        GIT_REPOSITORY https://github.com/ArthurSonzogni/nlohmann_json_cmake_fetchcontent
        GIT_PROGRESS TRUE
        GIT_SHALLOW TRUE
        GIT_TAG v3.11.2)
FetchContent_MakeAvailable(json)

add_library(mws OBJECT
        BasicDrainer.cpp
        BasicSolver.cpp
        BinomialHelper.cpp
        Drainer.cpp
        GameMgr.cpp
        random.cpp
        Solver.cpp
        facade.cpp)

if(EMSCRIPTEN)
    add_executable(MineSweeperSolver wasm.cpp)
else()
    find_package(Boost 1.80.0 REQUIRED)
    add_executable(MineSweeperSolver main.cpp)
    target_link_libraries(MineSweeperSolver PRIVATE mws)
    target_link_libraries(MineSweeperSolver PRIVATE nlohmann_json::nlohmann_json)
    target_link_libraries(MineSweeperSolver PRIVATE boost_thread)
    target_link_libraries(MineSweeperSolver PRIVATE pthread)
endif()

