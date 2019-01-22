CXX=${GCC_PATH}/g++
AR=ar
LIBS=-L ${BOOST_PATH}/stage/lib -lboost_program_options
CFLAGS=-I ${BOOST_PATH} --std=c++17 -Wall -Wno-sign-compare -Wno-parentheses -O3 -DVERSION=\"$$(git describe --always)\" -DCOMMITHASH=\"$$(git rev-parse HEAD)\"

SOLVER_TARGETS=BasicDrainer BasicSolver Drainer Solver GameMgr BinomialHelper ReadStrategy random
WORKER_TARGETS=BaseBaseWorker BaseWorker BaseWorkerT AdapterWorker Worker WorkerT main

.DEFAULT: all

all: build/worker

.PHONY: all clean

-include $(patsubst %, build/%.o.d, $(SOLVER_TARGETS))
-include $(patsubst %, build/%.o.d, $(WORKER_TARGETS))

build/MineSweeperSolver/%.o: MineSweeperSolver/%.cpp
	mkdir -p build/MineSweeperSolver
	$(CXX) -c -o $@ $< -MMD -MT $@ -MF $@.d $(CFLAGS)

build/MineSweeperSolver.a: $(patsubst %, build/MineSweeperSolver/%.o, $(SOLVER_TARGETS))
	mkdir -p build
	$(AR) rcs $@ $^

build/MineSweeperWorker/%.o: MineSweeperWorker/%.cpp
	mkdir -p build/MineSweeperWorker
	$(CXX) -c -o $@ $< -MMD -MT $@ -MF $@.d $(CFLAGS)

build/worker: $(patsubst %, build/MineSweeperWorker/%.o, $(WORKER_TARGETS)) build/MineSweeperSolver.a
	mkdir -p build
	$(CXX) -o $@ $(CFLAGS) $(LIBS) $^

clean:
	rm -rf build
