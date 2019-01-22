SELF_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
TARGETS=BasicDrainer BasicSolver Drainer Solver GameMgr BinomialHelper ReadStrategy random
DEPS=stdafx.h Strategies.h $(addsuffix .h, $(TARGETS))

.PHONY: clean $(SELF_DIR)-clean

clean: $(SELF_DIR)-clean

-include $(patsubst %, $(SELF_DIR)build/%.o.d, $(TARGETS))

$(SELF_DIR)build/%.o: $(SELF_DIR)%.cpp
	mkdir -p $(SELF_DIR)build
	$(CXX) -c -o $@ $< -MMD -MT $@ -MF $@.d $(CFLAGS)

$(SELF_DIR)build/MineSweeperSolver.a: $(patsubst %, $(SELF_DIR)build/%.o, $(TARGETS))
	mkdir -p $(SELF_DIR)build
	$(AR) rcs $@ $^

$(SELF_DIR)-clean:
	rm -rf $(SELF_DIR)build
