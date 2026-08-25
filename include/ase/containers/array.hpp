#pragma once

/**
 * =============================================================================
 * ASE CONTAINERS - ARRAY
 * =============================================================================
 *
 * @file        array.hpp
 * @brief       Array - fixed-size contiguous array (Foundation SSOT)
 * @description Single source of truth for fixed-capacity stack arrays. No heap
 *              allocation, size is a compile-time constant, and the storage is a
 *              plain C array so the type stays an aggregate: `Array<char, 32> x{};`
 *              zero-initialises, copies copy, and it may sit in a Component.
 *
 *              WAS ES BIS ZUM 2026-08-22 WAR: ein Alias auf den Standardtyp. Der
 *              Alias verdeckte die Kopplung, statt sie aufzuloesen — jede Datei, die
 *              `containers::Array` schrieb, zog den verbotenen Typ mit, und die
 *              Regel meldete ihn genau hier, im Fundament, wo er alle trifft.
 *
 * -----------------------------------------------------------------------------
 * DIE MENGE DER METHODEN, VORHER GEGEN NACHHER — gemessen, nicht geschaetzt
 * -----------------------------------------------------------------------------
 *              Baumweit gezaehlt wurde, was die Verbraucher an dem Typ WIRKLICH
 *              anfassen (25 Dateien, 36 Nennungen, Kommentare entfernt):
 *
 *                data()        34   Netzwerkpuffer, die an Sendefunktionen gehen
 *                operator[]    21
 *                size()        11
 *                fill()         1   `accumulators_.fill(0.0f)` in tick_scheduler.cpp
 *
 *              MEHR NICHT. Kein Bereichs-for, kein Vergleich, kein at/front/back/
 *              empty, keine Iteratoren, kein std::get, keine strukturierte Bindung,
 *              keine value_type/size_type. Jede dieser Formen wurde eigens gesucht,
 *              weil eine Punkt-Sonde sie nicht sieht: `for (x : a)` und `a == b`
 *              tragen keinen Methodennamen.
 *
 *              GETRAGEN werden hier: data, size, operator[], fill — und zusaetzlich
 *              begin/end, weil ein festes Feld sie geschenkt hergibt und ihr Fehlen
 *              genau die stille Sorte Bruch waere, die kein Tor meldet.
 *
 *              NICHT GETRAGEN, ausdruecklich und namentlich: at() (wirft, und
 *              Ausnahmen sind hier nicht die Fehlerform), front/back/empty/max_size/
 *              swap, Rueckwaerts-Iteratoren, die Vergleichsoperatoren und die
 *              Tupel-Schnittstelle (std::get, tuple_size). Fuer JEDE dieser Formen
 *              ist gemessen, dass sie heute im Baum null Rufer hat. Wer eine
 *              braucht, ergaenzt sie hier MIT seiner Aufrufstelle.
 *
 * -----------------------------------------------------------------------------
 * META
 * -----------------------------------------------------------------------------
 * @module      ase-containers
 * @layer       0 (Foundation)
 * @category    structure/container/sequential
 * @created     2026-06-06
 * @modified    2026-08-22
 * @version     2.0.0
 *
 * =============================================================================
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
