#pragma once

/**
 * ASE MODULE TYPES (SSOT)
 *
 * @file        types.hpp
 * @brief       Single Source of Truth for ase-containers constants and aliases
 * @description All compile-time constants and type aliases of the container
 *              layer. ALL module constants belong here - no magic numbers in
 *              the container headers themselves.
 *
 * @module      ase-containers
 * @layer       0 (Foundation)
 * @category    structure/container/sequential
 * @created     2026-01-07
 * @modified    2026-08-20
 * @version     1.1.0
 *
 * ECS TYPES COMPLIANCE
 *
 * [ ] All constants defined (no magic numbers in code)
 * [ ] Every constant has inline comment (English, explains purpose)
 * [ ] NO enum class (only constexpr uint8_t for enumeration values)
 * [ ] Type aliases defined
 * [ ] InvalidEntityId = UINT32_MAX defined (if needed)
 * [ ] Abbreviations documented
 * [ ] NO structs (structs belong in Components)
 */

#include <cstddef>
#include <cstdint>

namespace ase::containers {

/** RING BUFFER */

constexpr size_t RING_BUFFER_DEFAULT_CAPACITY = 1024;     // default slot count, power of two
constexpr size_t RING_BUFFER_MAX_CAPACITY = 1048576;      // upper bound, 1M entries

/** SLOT MAP */

constexpr size_t SLOT_MAP_DEFAULT_CAPACITY = 256;                 // initial slot count
constexpr uint32_t SLOT_MAP_MAX_GENERATION = UINT32_MAX - 1;      // last value before wrap-around

/** SMALL VECTOR */

constexpr size_t SMALL_VECTOR_DEFAULT_INLINE = 8;         // elements held without a heap allocation

/** TYPE ALIASES */

using Index = uint32_t;       // position of one element inside a container
using Size = uint32_t;        // element count of a container
using Generation = uint32_t;  // SlotMap generation counter, guards use-after-free

}  // namespace ase::containers
