CXX=${GCC_PATH}/g++
AR=ar
LIBS=-L ${BOOST_PATH}/stage/lib -lboost_program_options -lboost_system -lboost_thread
CFLAGS=-I ${BOOST_PATH} --std=c++17 -Wall -Wno-sign-compare -Wno-parentheses -O3 -pthread -DVERSION=\"$$(git describe --always)\" -DCOMMITHASH=\"$$(git rev-parse HEAD)\"

.DEFAULT: all

all:

include MineSweeperSolver/Makefile
include MineSweeperWorker/Makefile
