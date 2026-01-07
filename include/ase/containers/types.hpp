#pragma once

/**
 * =============================================================================
 * ASE CONTAINERS - TYPES (SSOT)
 * =============================================================================
 *
 * @file        types.hpp
 * @brief       Compile-time constants and type aliases for ase-containers
 * @description Single Source of Truth for container constants.
 *              ALL module constants belong here. NO magic numbers elsewhere!
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

#include <cstddef>
#include <cstdint>

namespace ase::containers {

// =============================================================================
// RingBuffer Constants
// =============================================================================

/// Default ring buffer capacity (must be power of 2)
constexpr size_t RING_BUFFER_DEFAULT_CAPACITY = 1024;

/// Maximum ring buffer capacity
constexpr size_t RING_BUFFER_MAX_CAPACITY = 1048576;  // 1M entries

// =============================================================================
// SlotMap Constants
// =============================================================================

/// Initial slot map capacity
constexpr size_t SLOT_MAP_DEFAULT_CAPACITY = 256;

/// Maximum generation value before wrap-around
constexpr uint32_t SLOT_MAP_MAX_GENERATION = UINT32_MAX - 1;

// =============================================================================
// SmallVector Constants
// =============================================================================

/// Default inline capacity for SmallVector
constexpr size_t SMALL_VECTOR_DEFAULT_INLINE = 8;

// =============================================================================
// Type Aliases
// =============================================================================

/// Index type for container operations
using Index = uint32_t;

/// Size type for container sizes
using Size = uint32_t;

/// Generation counter for SlotMap
using Generation = uint32_t;

}  // namespace ase::containers
