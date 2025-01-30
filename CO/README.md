# Combinatorial Optimisation

## Directory Structure

```bash
CO/
├── ABC_path_heuristic/        # Find the shortest path in the grid from A to C through B
├── sssp_exact/                # Single source shortest paths
├── min_matching_heuristic/    # Minimal maximum matching heuristic
├── min_matching_exact/        # Minimal maximum matching exact
└── radius_cover_heuristic/    # Radius vertex cover
```

## 1. Find the shortest path in the grid from A to C through B
### Problem definition
The problem is to find the shortest path without repeating nodes from A to C through B in the weighted grid. Where A is in the left bottom corner, B is in the right bottom corner, and C is in the left top corner.
```
C..
...
A.B
```

The arcs in the grid are weighted, here is how the grid looks like
```
C<->.<->.
^   ^   ^
|   |   |
v   v   v
.<->.<->.
^   ^   ^
|   |   |
v   v   v
A<->.<->B
```
Plus there are random arcs connecting nodes in columns with distance > 1.
### Our solution
Repeatedly run randomized Dijkstra algorithm from node A to B, and then run classic dijkstra from B to C.
Once the path from A to B is determined, the optimal path can be quickly determined with dijkstra algorithm.

## 2. Single source shortest paths
### Problem definition
Given a random, weighted and undirected graph and some queries (a, b), find the distance from a to b for each query
### Our solution
Find the distance for each query independently using bidirectional dijkstra

## 3. Minimal maximum matching heuristic
### Our solution
We are using 2 different greedy heuristics, and shuffle the vector with nodes multiple times, to introduce some randomness. In simple terms the first heuristic uses approximate greedy vertex cover to build the minimum maximal matching, and the second one:
```
// Idea - each edge leaves behind a crimson circle after it's chosen, nodes in crimson can't be chosen
// So I mark nodes adjacent to chosen edge and then they can't be chosen, until the third iteration in which
// I ensure the matching is maximal. Second iteration can be thougt as extension of the first one, I am using
// a number of iterations as offset for better exploration
```

## 4. Minimal maximum matching exact
### My solution
I am using branch & bound / reduce algorithm combined with many optimisations. 
- I am representing the graph as adjacency matrix which allows me to go through the entire algorithm without copying the graph.
- I use 2 simple reduction rules: if there is 1 edge - always add it to the solution, if there are 3 edges forming a line, take the middle edge if the matching cannot be made maximal from there - abandon this branch as it is suboptimal
- A non naive bounding condition:
```
// M is the current number of edges, the loop below finds an edge of highest degree
// Branch & Bound: if we have already exceeded the best known
// solution, don't search this branch further
// Adding an edge can remove itself and all adjacent edges, this reduces the degree of other edges,
// so (M / max) is number of edges needed to add to matching if all the added edges would have the
// same degree - max. When we add an edge to the solution, the degree of some nearby edges gets
// decreased but never increased. Therefore matching.size() + M / max is a lower bound on the
// size of matching, hence we can use it for branching.
```

## 5. Radius vertex cover
### Problem definition
Given some graph and an integer k, find the smallest set of nodes, such that each node in the graph is at most k edges away from the closest node in the solution.
### My solution
I am using 3 greedy heuristics, that are re-evaluated after updating the solution:
- Calculate how many times there is a node k edges away from the considered that is uncovered, and such that all its neighbors outside the k-range are already covered.
- Calculate how many nodes of degree 1 are in the range k of the considered node.
- Calculate how many uncovered nodes are in the range k from the considered node.

In order to efficiently update these heuristics I am collecting neighbours of the node added to solution in range 2 * k, and update the heuristcs only for them. Therefore I am recalculating the heuristics only for those nodes that could possibly be effected by the new node in the answer.

Once the first greedy solution is constructed, I am running local search with randomly removing parts of the solutions and filling them up with the above heuristics, until the time limit of 30s (actually to give some time for post processing 29s) is over.

## Contributors
- **[noNScop](https://github.com/noNScop)** - all the problems
- **[sppmacd](https://github.com/sppmacd)** - only first 3 problems
