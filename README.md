# Dynamic Network Routing Simulator

A C++17 command-line simulator that models how routers in a link-state
network (conceptually inspired by protocols like OSPF) build routing
tables, forward packets, and adapt to link failures in real time.

This is **not** an implementation of OSPF itself. It is a graph-based
teaching/demo tool that shows the underlying ideas — weighted graphs,
shortest-path computation, routing tables, and dynamic re-convergence —
using clean, modular, object-oriented C++.

---

## Overview

The simulator models a network as a weighted, undirected graph:

- **Routers** are nodes.
- **Links** are weighted edges (representing latency/cost).
- Each router computes shortest paths to every other router using
  **Dijkstra's algorithm**, exactly as a link-state routing protocol
  would compute its routing table from a link-state database.
- When a link **fails**, the network immediately reflects that in future
  path computations (failed links are excluded from Dijkstra's traversal).
  When it is **restored**, the network re-converges automatically —
  there is no separate "recompute" step, because routes are always
  computed on demand from the current topology.

Everything is driven through an interactive command-line interface.

---

## Features

- Add / remove routers at runtime.
- Add / remove / update weighted links between routers.
- Print the full network topology (adjacency list view).
- Compute and display the shortest path between any two routers.
- Generate a full routing table (Destination, Next Hop, Cost) for any router.
- Simulate packet forwarding between two routers, including delivery
  latency or a dropped-packet result when no path exists.
- Simulate link failures and restorations, with routing automatically
  adapting on the next lookup.
- Load and save network topologies from/to plain text files.
- Track running network statistics: total routers, total links,
  topology changes, packets sent, packets delivered, packets dropped,
  and average delivered latency.
- Friendly, defensive CLI that reports clear errors instead of crashing
  on malformed input.

---

## Folder Structure

```
NetworkRoutingSimulator/
├── include/
│   ├── Router.h        # Router node class
│   ├── Network.h       # Graph engine: topology, Dijkstra, routing tables, I/O
│   ├── Packet.h         # Packet data object used in forwarding simulation
│   ├── Edge.h            # Weighted edge structure
│   ├── PathResult.h       # Reusable shortest-path result structure
│   └── Simulator.h        # CLI driver / statistics tracker
├── src/
│   ├── Router.cpp
│   ├── Network.cpp
│   ├── Packet.cpp
│   ├── Simulator.cpp
│   └── main.cpp
├── data/
│   └── topology.txt     # Sample network topology
├── README.md
```

---

## Architecture

The project follows a clean separation of responsibilities:

| Class          | Responsibility                                                                 |
|-----------------|----------------------------------------------------------------------------------|
| `Router`        | Represents a single node's identity (id). Deliberately minimal.                  |
| `Edge`          | Represents one direction of a weighted link, including an `active` flag used for link-failure simulation. |
| `PathResult`    | A reusable result object returned by the shortest-path algorithm (path, total cost, reachability). |
| `Packet`        | Represents a single simulated packet and the outcome of its forwarding attempt (delivered / dropped, route taken, latency). |
| `Network`       | Owns the adjacency list, all topology-mutation operations, Dijkstra's algorithm, routing table generation, and file persistence. |
| `Simulator`     | Owns a `Network`, runs the interactive CLI loop, parses/dispatches commands, and tracks runtime statistics. |

**Key design decision:** the shortest-path algorithm
(`Network::computeShortestPath` / `computeAllShortestPaths`) returns a
`PathResult` (or a map of them) instead of printing anything directly.
This means the exact same Dijkstra computation is reused by three
different features without any duplicated logic:

1. `PATH` — prints a single computed route.
2. `TABLE` — builds a full routing table from `computeAllShortestPaths`.
3. `SEND` — uses the computed route to simulate packet forwarding and
   update statistics.

The graph itself is stored as `unordered_map<string, vector<Edge>>`
(an adjacency list). Every physical link is stored as two `Edge` entries
(one per direction), which keeps the graph conceptually undirected while
still allowing straightforward per-link failure/restoration bookkeeping.

---

## Algorithms Used

- **Dijkstra's Algorithm** (via `std::priority_queue` as a min-heap) —
  computes shortest paths from a source router to every other reachable
  router in O((V + E) log V) time. Failed links are simply skipped
  during edge relaxation, so a failure is automatically respected by
  every subsequent path computation without any extra state to manage.
- **Path reconstruction** — a `previous[]` map built during Dijkstra is
  walked backwards from destination to source to reconstruct the actual
  route, which is reversed into forward order.

---

## How to Compile

Requires `g++` with C++17 support.

```bash
make
```

This produces an executable named `router-sim` in the project root.

To remove build artifacts:

```bash
make clean
```

Or compile directly without the Makefile:

```bash
g++ -std=c++17 -Wall -Wextra -O2 -Iinclude src/*.cpp -o router-sim
```

---

## How to Run

```bash
./router-sim
```

You'll be dropped into an interactive prompt:

```
=====================================================
 Dynamic Network Routing Simulator
 Type HELP to see the list of available commands.
=====================================================

router-sim>
```

You can also load the bundled sample topology immediately:

```
router-sim> LOAD data/topology.txt
```

---

## Sample Commands

```
ADD R1
ADD R2
ADD R3
LINK R1 R2 5
LINK R2 R3 3
LINK R1 R3 9
PRINT
PATH R1 R3
TABLE R1
SEND R1 R3
FAIL R2 R3
PATH R1 R3
RESTORE R2 R3
LOAD data/topology.txt
SAVE data/my_topology.txt
STATS
HELP
EXIT
```

Full command reference:

| Command                        | Description                                              |
|----------------------------------|--------------------------------------------------------------|
| `ADD <router>`                   | Add a new router                                              |
| `REMOVE <router>`                 | Remove an existing router (and its links)                     |
| `LINK <a> <b> <cost>`             | Create a new weighted link between two routers                |
| `UNLINK <a> <b>`                  | Remove an existing link                                       |
| `UPDATE <a> <b> <cost>`           | Change the cost of an existing link                            |
| `PRINT`                           | Display the full topology                                     |
| `PATH <a> <b>`                    | Show the shortest path and cost between two routers            |
| `TABLE <router>`                  | Display the router's full routing table                        |
| `SEND <a> <b>`                    | Simulate forwarding a packet from a to b                        |
| `FAIL <a> <b>`                    | Simulate a link failure                                         |
| `RESTORE <a> <b>`                 | Restore a failed link                                           |
| `LOAD <file>`                     | Load a topology from a text file (replaces the current one)     |
| `SAVE <file>`                     | Save the current topology to a text file                        |
| `STATS`                           | Show network and packet statistics                              |
| `HELP`                            | Show the command list                                          |
| `EXIT` / `QUIT`                   | Exit the simulator                                              |

### Topology File Format

```
# Comments start with '#'
ROUTER R1
ROUTER R2
LINK R1 R2 5
```

---

## Future Improvements

- Support directed/asymmetric link costs (currently links are symmetric).
- Add a discrete-event simulation mode with simulated propagation delay
  and jitter rather than instantaneous cost-based delivery.
- Add alternate routing metrics (bandwidth, reliability) and support for
  multi-metric path selection.
- Persist runtime statistics across sessions (currently reset on restart).
- Add unit tests (e.g. via GoogleTest) covering `Network`'s graph
  operations and Dijkstra correctness on edge cases (disconnected graphs,
  self-loops, parallel-cost ties).
- Visualize the topology graphically (e.g. export to Graphviz DOT format).
