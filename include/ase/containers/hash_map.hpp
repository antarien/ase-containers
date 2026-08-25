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
 * @category    structure/container/associative
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
//
// Der dritte Parameter traegt den Hash-Functor durch — er existiert, damit
// Integer- und Verbund-Integer-Schluessel den vorgeschriebenen mischenden
// Functor `ase::containers::IntMixHash` setzen koennen (ARCH_ASE_HUB_ASYNC 9.7,
// MANDATORY invariant): auf libstdc++ ist `std::hash` fuer Ganzzahlen die
// Identitaet, und der Bucket kommt aus den unteren Bits — bei einem Verbund-
// Schluessel entscheidet dann nur EINE Haelfte, die andere kollidiert
// vollstaendig. Der Default bleibt `std::hash`, damit jede bestehende
// Verwendung mit zwei Parametern unveraendert gilt.
template <typename Key, typename Value, typename Hash = std::hash<Key>>
using HashMap = std::unordered_map<Key, Value, Hash>;

// Hashed unique-key set. Foundation SSOT alias for std::unordered_set.
// Dritter Parameter wie bei HashMap: Hash-Functor durchreichen, Default std::hash.
template <typename Key, typename Hash = std::hash<Key>>
using HashSet = std::unordered_set<Key, Hash>;

// Two-field tuple, paired with HashMap (map values are often (data, index) etc.).
template <typename First, typename Second>
using Pair = std::pair<First, Second>;

}  // namespace ase::containers
