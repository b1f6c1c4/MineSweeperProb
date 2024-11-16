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

set(CMAKE_EXPORT_COMPILE_COMMANDS YES)

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
    target_link_libraries(MineSweeperSolver PRIVATE embind)
else()
    find_package(Boost 1.81.0 REQUIRED)
    target_sources(mws PRIVATE Util.cpp)
    target_link_libraries(mws PUBLIC nlohmann_json::nlohmann_json)
    target_link_libraries(mws PUBLIC pthread)

    add_executable(MineSweeperSolver main.cpp)

    add_executable(MineSweeperProver Prover.cpp)
    target_link_libraries(MineSweeperProver PRIVATE mws)
    target_link_libraries(MineSweeperProver PRIVATE fmt)
    target_link_libraries(MineSweeperProver PRIVATE mimalloc)
    target_link_libraries(MineSweeperProver PRIVATE boost_chrono)
    target_link_libraries(MineSweeperProver PRIVATE boost_thread)
endif()

target_link_libraries(MineSweeperSolver PRIVATE mws)
