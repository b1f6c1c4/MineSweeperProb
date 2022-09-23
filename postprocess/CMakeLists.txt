cmake_minimum_required(VERSION 3.20)
project(postprocess)

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

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/CompilerAdditions)
include(WSTP-targets)

include(FetchContent)

set(JSON_MultipleHeaders OFF)
FetchContent_Declare(json
        GIT_REPOSITORY https://github.com/ArthurSonzogni/nlohmann_json_cmake_fetchcontent
        GIT_PROGRESS TRUE
        GIT_SHALLOW TRUE
        GIT_TAG v3.11.2)
FetchContent_MakeAvailable(json)

add_library(fmt fmt.cpp)

add_executable(postprocess main.cpp)
target_link_libraries(postprocess PRIVATE fmt)
target_link_libraries(postprocess PRIVATE WSTP::STATIC_LIBRARY)
target_link_libraries(postprocess PRIVATE nlohmann_json::nlohmann_json)

add_executable(test test.cpp test.cpp)
target_include_directories(test PRIVATE bandit)
target_link_libraries(test PRIVATE fmt)
