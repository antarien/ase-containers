#pragma once

/**
 * ASE CONTAINERS - ARRAY
 *
 * @file        array.hpp
 * @brief       Array - fixed-size contiguous array (Foundation SSOT)
 * @description Single source of truth for fixed-capacity stack arrays. No heap
 *              allocation, size is a compile-time constant, and the storage is a
 *              plain C array so the type stays an aggregate: `Array<char, 32> x{};`
 *              zero-initialises, assignment takes every element along, and it may
 *              sit in a Component.
 *
 * THE METHOD SET, MEASURED ACROSS THE TREE
 *
 *              Counted was what the consumers REALLY touch on the type (25 files,
 *              36 mentions, comments removed):
 *
 *                data()        34   network buffers handed to send functions
 *                operator[]    21
 *                size()        11
 *                fill()         1   `accumulators_.fill(0.0f)` in tick_scheduler.cpp
 *
 *              NOTHING ELSE. No range-for, no comparison, no at/front/back/empty, no
 *              iterators, no std::get, no structured binding, no value_type/size_type.
 *              Each of these forms was searched for on its own, because a dot probe
 *              does not see them: `for (x : a)` and `a == b` carry no method name.
 *
 *              CARRIED here: data, size, operator[], fill - and in addition
 *              begin/end, because a fixed field gives them for free and their absence
 *              would be exactly the silent kind of break no gate reports.
 *
 *              NOT CARRIED, explicitly and by name: at() (it throws, and exceptions
 *              are not the error form here), front/back/empty/max_size/swap, reverse
 *              iterators, the comparison operators and the tuple interface (std::get,
 *              tuple_size). For EACH of these forms it is measured that it has zero
 *              callers in the tree. Whoever needs one adds it here WITH the call site.
 *
 * META
 *
 * @module      ase-containers
 * @layer       0 (Foundation)
 * @category    structure/container/sequential
 * @usecase     usecase:101066 - accumulate values per owner
 * @actor       Foundation Container Steward (FOUNCONSTE)
 * @created     2026-06-06
 * @modified    2026-10-30
 * @version     2.0.1
 */

#include <cstddef>

namespace ase::containers {

/**
 * Fixed-capacity contiguous array.
 *
 * `items` is public and the type declares no constructor on purpose: that keeps it an
 * aggregate, so `Array<char, 16> a{};` value-initialises every element and the type
 * remains usable as plain Component data. Nested braces behave exactly as they did
 * before - `{{1, 2, 3}}` is the strict form, `{1, 2, 3}` works by brace elision.
 */
template <typename T, std::size_t N>
struct Array {
    static_assert(N > 0, "Array capacity must be greater than zero");

    T items[N];

    [[nodiscard]] constexpr T* data() noexcept { return items; }
    [[nodiscard]] constexpr const T* data() const noexcept { return items; }

    [[nodiscard]] constexpr std::size_t size() const noexcept { return N; }

    [[nodiscard]] constexpr T& operator[](std::size_t index) noexcept {
        return items[index];
    }
    [[nodiscard]] constexpr const T& operator[](std::size_t index) const noexcept {
        return items[index];
    }

    /// Overwrite every element with `value`.
    constexpr void fill(const T& value) {
        for (std::size_t i = 0; i < N; ++i) {
            items[i] = value;
        }
    }

    [[nodiscard]] constexpr T* begin() noexcept { return items; }
    [[nodiscard]] constexpr const T* begin() const noexcept { return items; }
    [[nodiscard]] constexpr T* end() noexcept { return items + N; }
    [[nodiscard]] constexpr const T* end() const noexcept { return items + N; }
};

}  // namespace ase::containers
