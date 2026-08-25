#pragma once

/**
 * =============================================================================
 * ASE CONTAINERS - ORDERED MAP / SET
 * =============================================================================
 *
 * @file        ordered.hpp
 * @brief       Map / Set - ordered associative containers (Foundation SSOT)
 * @description Single source of truth for sorted key lookup. Wraps the standard
 *              ordered associative containers so consumers depend on
 *              ase-containers, never on std:: directly (mirrors ase-json's
 *              `Json`). Use HashMap/HashSet when ordering is not required.
 *
 * -----------------------------------------------------------------------------
 * META
 * -----------------------------------------------------------------------------
 * @module      ase-containers
 * @layer       0 (Foundation)
 * @category    structure/container/associative
 * @created     2026-06-06
 * @modified    2026-06-06
 * @version     1.0.0
 *
 * =============================================================================
 */

#include <map>
#include <set>

namespace ase::containers {

// Sorted key/value map. Foundation SSOT alias for std::map.
template <typename Key, typename Value>
using Map = std::map<Key, Value>;

// Sorted unique-key set. Foundation SSOT alias for std::set.
template <typename Key>
using Set = std::set<Key>;

}  // namespace ase::containers
