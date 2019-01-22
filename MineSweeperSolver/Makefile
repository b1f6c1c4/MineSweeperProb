SELF_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
CXX=${GCC_PATH}/g++
AR=ar
TARGETS=BasicDrainer BasicSolver Drainer Solver GameMgr BinomialHelper random #main
DEPS=stdafx.h Strategies.h $(addsuffix .h, $(TARGETS))
CFLAGS=-I ${BOOST_PATH} --std=c++17 -Wall -Wno-sign-compare -Wno-parentheses -O3 -pthread -DVERSION=\"$$(git describe --always)\" -DCOMMITHASH=\"$$(git rev-parse HEAD)\"

.DEFAULT: all

.PHONY: all clean

all: $(SELF_DIR)build/MineSweeperSolver.a

-include $(patsubst %, $(SELF_DIR)build/%.o.d, $(TARGETS))

$(SELF_DIR)build/%.o: $(SELF_DIR)%.cpp
	mkdir -p $(SELF_DIR)build
	$(CXX) -c -o $@ $< -MMD -MT $@ -MF $@.d $(CFLAGS)

$(SELF_DIR)build/MineSweeperSolver.a: $(patsubst %, $(SELF_DIR)build/%.o, $(TARGETS))
	mkdir -p $(SELF_DIR)build
	$(AR) rcs $@ $^

clean:
	rm -rf $(SELF_DIR)build
