#pragma once

/**
 * ASE CONTAINERS - HASH MAP
 *
 * @file        hash_map.hpp
 * @brief       HashMap - unordered key/value map + Pair (Foundation SSOT)
 * @description Single source of truth for hashed key/value lookup. Wraps the
 *              standard unordered map so ECS Resource Managers and other
 *              consumers depend on ase-containers, never on std:: directly
 *              (the same wrapper form as ase-json's `Json`). Average-O(1)
 *              insert/find/erase. Pair is provided alongside since map values are
 *              frequently two-field tuples.
 *
 * META
 *
 * @module      ase-containers
 * @layer       0 (Foundation)
 * @category    structure/container/associative
 * @usecase     usecase:100837 - choose an index key hash
 * @actor       Foundation Container Steward (FOUNCONSTE)
 * @created     2026-06-06
 * @modified    2026-10-30
 * @version     1.0.1
 */

#include <unordered_map>
#include <unordered_set>
#include <utility>

namespace ase::containers {

// Hashed key/value map. Foundation SSOT alias so consumers never reach for
// std::unordered_map directly (same wrapper pattern as ase-json's Json).
//
// The third parameter carries the hash functor through - it exists so that integer
// and composite-integer keys can set the mandated mixing functor
// `ase::containers::IntMixHash` (ARCH_ASE_HUB_ASYNC 9.7, MANDATORY invariant): on
// libstdc++ `std::hash` for integers is the identity, and the bucket comes from the
// low bits - with a composite key only ONE half decides, the other collides
// completely. The default stays `std::hash`, so every use with two parameters keeps
// its meaning.
template <typename Key, typename Value, typename Hash = std::hash<Key>>
using HashMap = std::unordered_map<Key, Value, Hash>;

// Hashed unique-key set. Foundation SSOT alias for std::unordered_set.
// Third parameter as in HashMap: the hash functor passed through, default std::hash.
template <typename Key, typename Hash = std::hash<Key>>
using HashSet = std::unordered_set<Key, Hash>;

// Two-field tuple, paired with HashMap (map values are often (data, index) etc.).
template <typename First, typename Second>
using Pair = std::pair<First, Second>;

}  // namespace ase::containers
