#pragma once

/**
 * =============================================================================
 * ASE CONTAINERS - HASH MAP
 * =============================================================================
 *
 * @file        hash_map.hpp
 * @brief       HashMap - unordered key/value map + Pair (Foundation SSOT)
 * @description Single source of truth for hashed key/value lookup. Wraps the
 *              standard unordered map so ECS Resource Managers and other
 *              consumers depend on ase-containers, never on std:: directly
 *              (mirrors ase-json's `Json`). Average-O(1) insert/find/erase.
 *              Pair is provided alongside since map values are frequently
 *              two-field tuples.
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

#include <unordered_map>
#include <unordered_set>
#include <utility>

namespace ase::containers {

// Hashed key/value map. Foundation SSOT alias so consumers never reach for
// std::unordered_map directly (same wrapper pattern as ase-json's Json).
template <typename Key, typename Value>
using HashMap = std::unordered_map<Key, Value>;

// Hashed unique-key set. Foundation SSOT alias for std::unordered_set.
template <typename Key>
using HashSet = std::unordered_set<Key>;

// Two-field tuple, paired with HashMap (map values are often (data, index) etc.).
template <typename First, typename Second>
using Pair = std::pair<First, Second>;

}  // namespace ase::containers
