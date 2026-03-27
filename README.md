# ase-containers

[![Layer](https://img.shields.io/badge/Layer-0%20Foundation-blue.svg)]()
[![C++20](https://img.shields.io/badge/C%2B%2B-20-blue.svg)]()
[![Status](https://img.shields.io/badge/Status-Planned-yellow.svg)]()

> High-performance data structures optimized for game engine workloads

Part of [ASE - Antares Simulation Engine](../../..)

## Overview

The ase-containers module provides high-performance data structures optimized for game engine patterns, replacing standard library containers where cache locality, lock-free access, or generational safety are required. Standard containers like std::vector, std::map, and std::unordered_map are general-purpose but introduce cache misses, heap fragmentation, and iterator invalidation problems that are unacceptable in a real-time ECS simulation running at 30-60 Hz. Planned containers include SparseSet for dense iteration over sparse entity data, SlotMap with generational indices to prevent use-after-free on destroyed entities, RingBuffer for lock-free single-producer/single-consumer communication between threads, FlatMap for cache-friendly hash lookups with open addressing, and SmallVector with stack-based small-size optimization to avoid heap allocation for typical component counts. As a Layer 0 foundation library, ase-containers has no ASE dependencies and provides the same guarantees as standard containers (RAII, exception safety) while being optimized for the access patterns that dominate ECS workloads: sequential iteration, random access by entity ID, and high-frequency insert/remove cycles. Each container is designed for a specific ECS access pattern — SparseSet for component storage iteration at near-array speed, SlotMap for stable entity handles that detect use-after-destroy, and RingBuffer for thread-safe message passing between the game loop and async I/O workers.

## Planned Features

- **SparseSet**: Fast iteration with stable indices (ECS component storage)
- **SlotMap**: Generational indices to prevent use-after-free
- **RingBuffer**: Lock-free circular buffer for producer-consumer patterns
- **FlatMap**: Cache-friendly hash map with open addressing
- **StaticVector**: Fixed-capacity vector without heap allocation
- **SmallVector**: Stack-optimized vector with small-size optimization

## Motivation

Standard containers (`std::vector`, `std::map`, etc.) are general-purpose but not optimized for game engine patterns:

- ECS systems need fast iteration over sparse data (SparseSet)
- Entity handles need generational indices to detect stale references (SlotMap)
- Lock-free communication between threads (RingBuffer)
- Frequent lookups without allocation overhead (FlatMap)
- Small collections without heap fragmentation (SmallVector)

## Planned Usage

```cpp
// SparseSet for ECS components (not yet implemented)
#include <ase/containers/sparse_set.hpp>

using namespace ase::containers;

SparseSet<Position> positions;
positions.insert(entity_id, Position{x, y, z});

// Fast iteration (dense array)
for (auto& pos : positions) {
    // Cache-friendly iteration
}

// O(1) lookup
if (auto* pos = positions.get(entity_id)) {
    // Entity has position
}

// SlotMap for safe handles (not yet implemented)
#include <ase/containers/slot_map.hpp>

SlotMap<Texture> textures;
auto handle = textures.insert(load_texture("terrain.png"));

// Safe access - returns nullptr if texture was deleted
if (auto* tex = textures.get(handle)) {
    render(*tex);
}

// RingBuffer for lock-free queues (not yet implemented)
#include <ase/containers/ring_buffer.hpp>

RingBuffer<Event, 1024> event_queue;

// Producer thread
event_queue.push(Event{...});

// Consumer thread
if (auto event = event_queue.pop()) {
    handle_event(*event);
}
```

## Dependencies

### External
- C++20 standard library

### Internal
- None (Layer 0 - Foundation)

## References

- [EnTT SparseSet](https://github.com/skypjack/entt/wiki/Crash-Course:-entity-component-system#sparse-sets)
- [SlotMap (Rust)](https://docs.rs/slotmap/latest/slotmap/)
- [Lock-Free Ring Buffer](https://www.codeproject.com/Articles/43510/Lock-Free-Single-Producer-Single-Consumer-Circular)

## Contributing

This module is planned but not yet implemented. If you need these containers:
1. Use STL containers as temporary solution
2. Implement containers in this module following Layer 0 guidelines
3. Ensure no dependencies on other ASE modules
4. Add comprehensive unit tests

## License

Proprietary - ASE Engine

---

**Layer 0 Foundation** | No ASE dependencies | Planned
