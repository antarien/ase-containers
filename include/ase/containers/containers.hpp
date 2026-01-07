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
 *
 *              Planned:
 *                SlotMap<T>            - Generational indices (use-after-free safe)
 *                FlatMap<K, V>         - Cache-friendly hash map
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
