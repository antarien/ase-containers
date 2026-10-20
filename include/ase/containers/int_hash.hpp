#pragma once

/**
 * ASE CORE INFRASTRUCTURE HEADER
 *
 * @file        int_hash.hpp
 * @brief       IntMixHash - streuender Hash-Functor fuer Integer-Schluessel (Foundation SSOT)
 * @description Pflicht-Hash fuer JEDE libcuckoo::cuckoohash_map mit einem
 *              Integer- oder Verbund-Schluessel (uint32_t / uint64_t /
 *              uintptr_t). Der Default std::hash<Integer> ist auf libstdc++ die
 *              IDENTITAET, und libcuckoo waehlt den Primaer-Bucket ueber die
 *              UNTEREN Bits (hv & hashmask). Sitzt die Entropie eines
 *              Verbund-Schluessels in den OBEREN Bits — etwa (owner<<32)|value_id,
 *              (magic<<32)|ordinal, (discriminator<<32)|counter —, fallen die
 *              oberen Bits aus der Bucket-Wahl heraus: alle Schluessel, die sich
 *              nur oben unterscheiden, kollidieren auf DEMSELBEN 4-Slot-Bucket.
 *              Die Cuckoo-Kicks laufen leer, die Map fordert eine Expansion bei
 *              fast leerer Tabelle, libcuckoo wirft load_factor_too_low, niemand
 *              faengt es, std::terminate beendet den Prozess.
 *
 *              Der Hub-Verbund-Schluessel (owner<<32)|value_id mit vielen Ownern
 *              je value_id kollabiert genau so und beendet die Engine mit Exit 134.
 *              Die Reasoning-Request-Indizes (vault/memory/quota) tragen dasselbe
 *              Muster (getaggte 64-Bit-Ids, siehe VLT_SEED_REQ_ID = 0x5EED...0024).
 *
 *              splitmix64-Finalizer: mischt ALLE Bits in die unteren, sodass
 *              owner UND value_id (bzw. jede Komponente eines Verbund-Schluessels)
 *              den Bucket bestimmen. Die Verteilung ist damit unabhaengig davon,
 *              wie viele Eintraege sich nur in den oberen Bits unterscheiden —
 *              die Voraussetzung fuer horizontale Skalierung (viele Owner je Key).
 *
 *              WICHTIG: aendert NUR die Bucket-Platzierung, NIE den gespeicherten
 *              Schluessel. `map.find(k)`/`insert(k,v)` bleiben bitgenau; erlaubt
 *              per PLAN_ASE_COMPUTE.md:246 (kein make_owner/Owner-Bit-Tagging am
 *              gespeicherten Key — hier wird der Key nicht angefasst).
 *
 *              Ein Schluessel aus Text oder Bytes bekommt keinen Hash aus dieser
 *              Bibliothek: dafuer steht entt::hashed_string (FNV-1a 32 Bit) im Baum.
 *
 * @module      ase-containers
 * @layer       0 (Foundation)
 * @category    structure/container/associative
 * @usecase     usecase:100837 - choose an index key hash
 * @created     2026-07-16
 * @modified    2026-10-15
 * @version     1.0.1
 *
 * LAYER RULES:
 *   Layer 0 (Foundation): NO dependencies on other ASE modules (only std::)
 *
 * USAGE:
 *   #include <ase/containers/int_hash.hpp>
 *   libcuckoo::cuckoohash_map<uint64_t, V, ase::containers::IntMixHash> map;
 *
 * CORE INFRASTRUCTURE COMPLIANCE
 *
 * [ ] NOT an ECS Component or System
 * [ ] Layer dependencies correct (L0: no ASE deps, L1: L0 only)
 * [ ] No global mutable state (constexpr/const only)
 * [ ] No singletons or static mutable variables
 * [ ] Thread-safe by design (pure functions or explicit mutex)
 * [ ] All public functions documented with @brief, @param, @return
 * [ ] constexpr where possible (compile-time evaluation)
 * [ ] noexcept where possible (no-throw guarantee)
 * [ ] [[nodiscard]] on functions returning values
 * [ ] No magic numbers (use named constants)
 * [ ] No implicit conversions (use explicit constructors)
 * [ ] Header-only OR header+cpp pattern (not mixed)
 * [ ] Include guards via #pragma once
 * [ ] Namespace matches module: ase::{module}
 * [ ] No circular dependencies
 * [ ] No macros (except include guards) - use constexpr/templates
 * [ ] API stable (changes require version bump)
 */

#include <cstddef>
#include <cstdint>

namespace ase::containers {

/**
 * Streuender Hash-Functor fuer Integer- und Verbund-Integer-Schluessel.
 *
 * Als 3. Template-Parameter jeder libcuckoo::cuckoohash_map mit Integer-Key zu
 * setzen:  cuckoohash_map<uint64_t, V, ase::containers::IntMixHash>.
 *
 * splitmix64 (Steele/Vigna) — Bijektion, damit keine kuenstlichen Kollisionen
 * entstehen; der Finalizer verteilt jede Bit-Position gleichmaessig auf die
 * unteren Bits, die libcuckoo fuer die Bucket-Wahl liest.
 */
struct IntMixHash {
    std::size_t operator()(std::uint64_t k) const noexcept {
        k ^= k >> 30;
        k *= 0xbf58476d1ce4e5b9ULL;
        k ^= k >> 27;
        k *= 0x94d049bb133111ebULL;
        k ^= k >> 31;
        return static_cast<std::size_t>(k);
    }
};

}  // namespace ase::containers
