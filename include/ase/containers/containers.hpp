#pragma once

/**
 * =============================================================================
 * ASE CONTAINERS - MAIN INCLUDE
 * =============================================================================
 *
 * @file        containers.hpp
 * @brief       Main include for ase-containers module
 * @description High-performance data structures optimized for game engine
 *              workloads. Provides ECS-friendly containers with cache-optimal
 *              iteration and lock-free multi-threaded patterns.
 *
 *              Containers:
 *                SparseSet<T>          - O(1) insert/remove/lookup, O(n) iteration
 *                RingBuffer<T, N>      - Lock-free SPSC circular buffer
 *                Vector<T>             - Growable contiguous array (SSOT)
 *                HashMap<K, V>         - Hashed key/value map (SSOT)
 *                HashSet<K>            - Hashed unique-key set (SSOT)
 *                Map<K, V>             - Ordered key/value map (SSOT)
 *                Set<K>                - Ordered unique-key set (SSOT)
 *                Array<T, N>           - Fixed-size contiguous array (SSOT)
 *                Deque<T>              - Double-ended queue (SSOT)
 *                List<T>               - Doubly-linked list (SSOT)
 *                Pair<A, B>            - Two-field tuple (SSOT)
 *
 *              Planned:
 *                SlotMap<T>            - Generational indices (use-after-free safe)
 *                FlatMap<K, V>         - Cache-friendly open-addressing map
 *                StaticVector<T, N>    - Fixed-capacity, no heap allocation
 *                SmallVector<T, N>     - Stack-optimized with SSO
 *
 * -----------------------------------------------------------------------------
 * META
 * -----------------------------------------------------------------------------
 * @module      ase-containers
 * @layer       0 (Foundation)
 * @created     2026-01-07
 * @modified    2026-01-07
 * @version     1.0.0
 *
 * =============================================================================
 */

// =============================================================================
// Types (SSOT)
// =============================================================================
#include <ase/containers/types.hpp>

// =============================================================================
// Containers
// =============================================================================
#include <ase/containers/ring_buffer.hpp>
#include <ase/containers/vector.hpp>
#include <ase/containers/hash_map.hpp>
#include <ase/containers/ordered.hpp>
#include <ase/containers/array.hpp>
#include <ase/containers/sequence.hpp>
