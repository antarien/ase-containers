#pragma once

/**
 * =============================================================================
 * ASE CONTAINERS - INTEGER MIX HASH
 * =============================================================================
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
 *              LIVE 2026-07-16: Engine exit 134 direkt nach "Almanach sent
 *              (21983 entries)" — der Hub-Verbund-Schluessel (owner<<32)|value_id
 *              mit vielen Ownern je value_id kollabierte genau so. Die Reasoning-
 *              Request-Indizes (vault/memory/quota) tragen dasselbe Muster
 *              (getaggte 64-Bit-Ids, siehe VLT_SEED_REQ_ID = 0x5EED...0024).
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
 * -----------------------------------------------------------------------------
 * META
 * -----------------------------------------------------------------------------
 * @module      ase-containers
 * @layer       0 (Foundation)
 * @category    structure/container/associative
 * @created     2026-07-16
 * @modified    2026-07-16
 * @version     1.0.0
 *
 * =============================================================================
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
