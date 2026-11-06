#pragma once

/**
 * ASE CORE INFRASTRUCTURE HEADER
 *
 * @file        containers.hpp
 * @design      DSGN_016
 * @brief       Main include for ase-containers module
 * @description High-performance data structures optimized for game engine
 *              workloads. Provides ECS-friendly containers with cache-optimal
 *              iteration and lock-free multi-threaded patterns. One include brings every
 *              container and the hash they are instantiated with;
 *              tests/containers_umbrella_test.cpp includes this header alone and drives each type.
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
 *              Hashing:
 *                IntMixHash            - Scattering hash for integer and composite integer keys
 *                                        (usecase:100837 choose an index key hash)
 *
 *              No Deque, no List and no SparseSet: a queue is RingBuffer (the remedy
 *              STD_DEQUE_FORBIDDEN names), a list of N things is one entity per item (the
 *              remedy STD_LIST_FORBIDDEN names).
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
 * @usecase     usecase:100035 - compile used headers
 * @actor       Tier Build Orchestrator (TIERBUIORC)
 * @created     2026-01-07
 * @modified    2026-10-31
 * @version     1.2.1
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

// Types (SSOT)
#include <ase/containers/types.hpp>

// Containers
#include <ase/containers/ring_buffer.hpp>
#include <ase/containers/vector.hpp>
#include <ase/containers/hash_map.hpp>
#include <ase/containers/ordered.hpp>
#include <ase/containers/array.hpp>

// Hashing
#include <ase/containers/int_hash.hpp>

namespace ase::containers {

// Declarations live in the headers included above.

}  // namespace ase::containers
