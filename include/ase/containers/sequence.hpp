#pragma once

/**
 * =============================================================================
 * ASE CONTAINERS - SEQUENCE (DEQUE / LIST)
 * =============================================================================
 *
 * @file        sequence.hpp
 * @brief       Deque / List - node/chunk sequence containers (Foundation SSOT)
 * @description Single source of truth for double-ended and linked sequences.
 *              Wraps the standard sequence containers so consumers depend on
 *              ase-containers, never on std:: directly (mirrors ase-json's
 *              `Json`). Prefer Vector unless front-insertion (Deque) or stable
 *              node addresses (List) are required.
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

#include <deque>
#include <list>

namespace ase::containers {

// Double-ended queue (O(1) push/pop at both ends). SSOT alias for std::deque.
template <typename T>
using Deque = std::deque<T>;

// Doubly-linked list (stable element addresses). SSOT alias for std::list.
template <typename T>
using List = std::list<T>;

}  // namespace ase::containers
