SELF_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
TARGETS=BaseBaseWorker BaseWorker BaseWorkerT AdapterWorker Worker WorkerT main
DEPS=stdafx.h Configuration.h $(addsuffix .h, $(TARGETS))

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
