SELF_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
CXX=${GCC_PATH}/g++
AR=ar
TARGETS=BaseBaseWorker BaseWorker BaseWorkerT Worker WorkerT main
LIBS=-L ${BOOST_PATH}/stage/lib -lboost_program_options -lboost_system -lboost_thread
DEPS=stdafx.h Configuration.h $(addsuffix .h, $(TARGETS))
CFLAGS=-I ${BOOST_PATH} --std=c++17 -Wall -Wno-sign-compare -Wno-parentheses -O3 -pthread -DVERSION=\"$$(git describe --always)\" -DCOMMITHASH=\"$$(git rev-parse HEAD)\"

.DEFAULT: all

.PHONY: all clean $(SELF_DIR)-clean

clean: $(SELF_DIR)-clean

all: $(SELF_DIR)build/MineSweeperWorker

-include $(patsubst %, $(SELF_DIR)build/%.o.d, $(TARGETS))

$(SELF_DIR)build/%.o: $(SELF_DIR)%.cpp
	mkdir -p $(SELF_DIR)build
	$(CXX) -c -o $@ $< -MMD -MT $@ -MF $@.d $(CFLAGS)

$(SELF_DIR)build/MineSweeperWorker: $(patsubst %, $(SELF_DIR)build/%.o, $(TARGETS)) $(SELF_DIR)../MineSweeperSolver/build/MineSweeperSolver.a
	mkdir -p $(SELF_DIR)build
	$(CXX) -o $@ $(CFLAGS) $(LIBS) $^

$(SELF_DIR)-clean:
	rm -rf $(SELF_DIR)build
