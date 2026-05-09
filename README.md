# 🌲 Forest Fire — Self-Organized Criticality

> **A single lightning strike can ignite a wildfire that burns a thousand acres.**
> The Butterfly Effect in ecology.

## Concept

A 2D cellular automaton demonstrating **Self-Organized Criticality (SOC)** — the system naturally evolves toward a critical state where events of all sizes occur, from single-tree flickers to catastrophic wildfires.

## Rules

| State | Transition | Condition |
|-------|-----------|-----------|
| 🌑 Empty | → 🌲 Tree | Probability `p` per step (growth) |
| 🌲 Tree | → 🔥 Burn | Neighbor burning OR lightning `f` |
| 🔥 Burn | → 🌑 Empty | After 1 step |

## Butterfly Effect

- **Small cause, massive effect**: One lightning bolt → one burning tree → spreads to neighbors → cascade → thousand-acre fire
- **Sensitive to initial conditions**: Same parameters → different fire patterns each run
- **Power-law distribution**: `N(fires of size s) ∝ s^(-α)` — fires of all sizes, few giant + many small

## Self-Organized Criticality

Bak, Tang & Wiesenfeld (1987) showed forest fire models exhibit SOC:
1. System evolves toward critical state without tuning parameters
2. Disturbances (fires) of all sizes occur
3. Power-law size distribution (α ≈ 1.0–2.0)

## Build

```bash
mkdir build && cd build
cmake .. && make
./forest_fire
```

Requires: SFML (for graphics) or compiles as ASCII-only fallback.

## Controls

- **R** — Reset simulation
- **Space** — Step one frame
- **Click** — Manual ignition (add fire at cursor)
- **Esc** — Quit

## Physics

- **Cellular automaton** on toroidal grid (periodic boundary)
- **Contagion**: fire spreads to 8-connected neighbors
- **Spontaneous ignition**: rare random lightning strikes
- **Tree regrowth**: slow probability-based

## Parameters (hardcoded)

```cpp
P_GROW = 0.01    // tree growth rate
F_LIGHT = 0.0002 // lightning probability
GRID = 120×80    // cells × 8px
```

## Butterfly Effect in This Model

```
Lightning (1 tree) → BURN → spreads to 8 neighbors
                             → each neighbor spreads to 8 more
                                   → cascade continues...
                                   → 10,000 trees burned
```

The tiniest spark (1 cell) can, if the conditions are right (dry forest = high tree density), grow into a fire that consumes the entire forest. This is the butterfly effect in ecological form: small perturbation → disproportionate consequence.

## Why C++

- **Performance**: 120×80 grid, 30+ FPS, full BFS fire cluster detection every frame
- **Control**: fine-grained cellular automaton rules, deterministic by design
- **SFML**: hardware-accelerated 2D rendering

---

*Part of kuuila/daily — one creative coding project per day.*
