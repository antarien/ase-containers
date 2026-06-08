#pragma once

/**
 * =============================================================================
 * ASE CONTAINERS - ARRAY
 * =============================================================================
 *
 * @file        array.hpp
 * @brief       Array - fixed-size contiguous array (Foundation SSOT)
 * @description Single source of truth for fixed-capacity stack arrays. Wraps the
 *              standard fixed array so consumers depend on ase-containers, never
 *              on std:: directly (mirrors ase-json's `Json`). No heap allocation;
 *              size is a compile-time constant.
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

#include <array>
#include <cstddef>

namespace ase::containers {

// Fixed-capacity contiguous array. Foundation SSOT alias for std::array.
template <typename T, std::size_t N>
using Array = std::array<T, N>;

}  // namespace ase::containers
