#pragma once

/**
 * =============================================================================
 * ASE CONTAINERS - VECTOR
 * =============================================================================
 *
 * @file        vector.hpp
 * @brief       Vector - dynamic contiguous array (Foundation SSOT)
 * @description Single source of truth for growable contiguous arrays. Wraps the
 *              standard dynamic array so that ECS Resource Managers and other
 *              consumers depend on ase-containers, never on std:: directly
 *              (mirrors ase-json's `Json`). Heap-backed, contiguous storage,
 *              amortised-O(1) append, random access.
 *
 * -----------------------------------------------------------------------------
 * META
 * -----------------------------------------------------------------------------
 * @module      ase-containers
 * @layer       0 (Foundation)
 * @created     2026-06-06
 * @modified    2026-06-06
 * @version     1.0.0
 *
 * =============================================================================
 */

#include <vector>

namespace ase::containers {

// Growable contiguous array. Foundation SSOT alias so consumers never reach
// for std::vector directly (same wrapper pattern as ase-json's Json).
template <typename T>
using Vector = std::vector<T>;

}  // namespace ase::containers
