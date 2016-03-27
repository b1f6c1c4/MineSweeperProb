# MineSweeperProb
A deterministic Minesweeper solver

# Projects
## MineSweeperSolver
A configurable core Minesweeper solver written in C++.

## MineSweeper
An interactive UI written in C#, invoking the core solver using P/Invoke.

## MineSweeperDemo
A non-interactive demo for the core written in C++, invoking the core solver.

## Simulator(s)
A distributed testing platform used to estimate the success rates of different strategies.
### MineSweeperSimulator
For conducting Monte-Carlo experiments, written in C++.
### SimulatorManagementClient
For monitoring MineSweeperSimulator and communicating with SimulatorsManager, written in C#.
### SimulatorsManager
A centralized management console, communicating with some 
SimulatorManagementClient.

## MineSweeperSpeed
A testing platform used to estimate the time consumption of different strategies, written in C++.

## MineSweeperProver
A program used to find the upper-bound of all possible Minesweeper strategies.

# Branches
## double
The solver implementation internally uses `double` to calculate. Thus, the grid size can not be too large. It's enough for the expert-level game (99 mines over a 30 * 16 grid) or smaller, but not the "super" - (720 mines over an 83 * 40 grid).

## master
The solver implementation internally uses high-precision calculation. The scale limitation is the memory and time limitation. However, it leads to increased time consumption.

# Strategies
A strategy consists of three algorithms: an initial algorithm, a logical algorithm, and a conjectural algorithm. The initial algorithm is used to determine the first move. The logical algorithm is used to deduce *safe* blocks. The conjectural algorithm is used to making guesses when the logical algorithm fails.
`struct Strategy` is where to the configure the solver. Let `InitialPositionSpecified = true` and assign a number to `Index` to specify the initial algorithm. Use `Logic` to specify the logical algorithm. Use `HeuristicEnabled` and `DecisionTree` to specify the conjectural algorithm. Moreover, enable `ExhaustEnabled` and choose appropriate `ExhaustCriterion` can bring about significant improvements in the success rate. However, time consumption blooms as well, which may be cut down by `PruningEnabled`, `PruningCriterion` and `PruningDecisionTree`.

# Usage
You may refer to `MineSweeperSimulator` and `MineSweeperDemo` for how to invoke the solver in C++, and `MineSweeper` for that in C#.
