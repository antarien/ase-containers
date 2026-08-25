#pragma once

/**
 * ASE CORE INFRASTRUCTURE HEADER
 *
 * @file        containers.hpp
 * @brief       Main include for ase-containers module
 * @description High-performance data structures optimized for game engine
 *              workloads. Provides ECS-friendly containers with cache-optimal
 *              iteration and lock-free multi-threaded patterns.
 *
 *              Containers:
 *                RingBuffer<T, N>      - Lock-free SPSC circular buffer
 *                Vector<T>             - Growable contiguous array (SSOT)
 *                HashMap<K, V>         - Hashed key/value map (SSOT)
 *                HashSet<K>            - Hashed unique-key set (SSOT)
 *                Map<K, V>             - Ordered key/value map (SSOT)
 *                Set<K>                - Ordered unique-key set (SSOT)
 *                Array<T, N>           - Fixed-size contiguous array (SSOT)
 *                Pair<A, B>            - Two-field tuple (SSOT)
 *
 *              REMOVED 2026-08-20, and the reason is written here rather than left to be
 *              re-derived, because a list of containers is exactly where a deleted type
 *              goes on being advertised:
 *                SparseSet<T>  - had no header at all. Deleted 2026-01-07 by commit
 *                                1ccd266 "Remove sparse_set, keep ring_buffer only"; this
 *                                block kept listing it among the PRESENT types for seven
 *                                months.
 *                Deque<T>      - measured tree-wide: 0 code users, 0 mentions. sequence.hpp
 *                List<T>         was included by exactly one file (this one), and this one
 *                                by nobody. std::deque and std::list appear only inside
 *                                validator fixtures, so the need does not exist either -
 *                                and the forbidding rules point elsewhere for the remedy:
 *                                STD_DEQUE_FORBIDDEN says "Use ase::containers::RingBuffer",
 *                                STD_LIST_FORBIDDEN says "Use Entity-per-Item Pattern".
 *                                Neither named these aliases.
 *
 *              Planned:
 *                SlotMap<T>            - Generational indices (use-after-free safe)
 *                FlatMap<K, V>         - Cache-friendly open-addressing map
 *                StaticVector<T, N>    - Fixed-capacity, no heap allocation
 *                SmallVector<T, N>     - Stack-optimized with SSO
 *
 * @module      ase-containers
 * @layer       0 (Foundation)
 * @category    structure/container/sequential
 * @created     2026-01-07
 * @modified    2026-08-20
 * @version     1.1.0
 *
 * CORE INFRASTRUCTURE COMPLIANCE
 *
 * [ ] NOT an ECS Component or System
 * [ ] Layer dependencies correct (L0: no ASE deps, L1: L0 only)
 * [ ] No global mutable state (constexpr/const only)
 * [ ] No singletons or static mutable variables
 * [ ] Thread-safe by design (pure functions or explicit mutex)
 * [ ] All public functions documented with @brief, @param, @return
 * [ ] constexpr where possible (compile-time evaluation)
 * [ ] noexcept where possible (no-throw guarantee)
 * [ ] [[nodiscard]] on functions returning values
 * [ ] No magic numbers (use named constants)
 * [ ] No implicit conversions (use explicit constructors)
 * [ ] Header-only OR header+cpp pattern (not mixed)
 * [ ] Include guards via #pragma once
 * [ ] Namespace matches module: ase::{module}
 * [ ] No circular dependencies
 * [ ] No macros (except include guards) - use constexpr/templates
 * [ ] API stable (changes require version bump)
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

namespace ase::containers {

// Declarations live in the headers included above.

}  // namespace ase::containers
