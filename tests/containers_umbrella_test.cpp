/**
 * ASE CORE INFRASTRUCTURE IMPLEMENTATION
 *
 * @file        containers_umbrella_test.cpp
 * @design      DSGN_016
 * @brief       Compiles the module header containers.hpp and drives every container it offers
 * @description containers.hpp is the one include a caller takes to get every container of the
 *              module. It is the entry the module's CMakeLists names first, and until this file
 *              no translation unit included it: every user took the single headers, so a
 *              broken umbrella would have stayed green in every build.
 *
 *              THIS FILE INCLUDES NOTHING OF THE MODULE BUT THE UMBRELLA. Every type below is
 *              reached through it; a container the umbrella stops pulling in fails here at
 *              compile time, not in the first caller that switches to the one include.
 *
 *              The cases follow what the containers are for in the foundation:
 *              index entities by composite key (usecase:101065) - HashMap and HashSet with the
 *              scattering hash IntMixHash; accumulate values per owner (usecase:101066) - Map
 *              and Set in key order, Vector and Pair; the fixed Array and the RingBuffer whose
 *              full and empty edges ring_buffer_test.cpp covers in depth.
 *
 *              ITS OWN TARGET, NOT A SECOND SOURCE OF ase-containers-test: that target switches
 *              the doctest entry point on for its one translation unit (ring_buffer_test.cpp says
 *              so), and a second source would bring a second main() that breaks only at link time.
 *
 * @module      ase-containers
 * @layer       0 (Foundation)
 * @category    process/validation/check
 * @usecase     usecase:100035 - compile used headers
 * @actor       Tier Build Orchestrator (TIERBUIORC)
 * @created     2026-10-30
 * @modified    2026-10-31
 * @version     1.0.1
 *
 * CORE INFRASTRUCTURE IMPLEMENTATION COMPLIANCE
 *
 * [ ] NOT an ECS System implementation
 * [ ] Layer dependencies correct (L0: no ASE deps, L1: L0 only)
 * [ ] Own header included FIRST
 * [ ] No global mutable state
 * [ ] No static initialization order fiasco
 * [ ] Thread-safe implementations (pure or mutex-protected)
 * [ ] All error conditions handled
 * [ ] No exceptions thrown (use Result<T> pattern)
 * [ ] Implementation details in anonymous namespace
 * [ ] No inline implementations of template specializations here
 * [ ] Platform-specific code isolated and documented
 * [ ] Performance-critical code profiled and optimized
 */

#include <ase/containers/containers.hpp>

#include <doctest/doctest.h>

#include <cstdint>

using ase::containers::Array;
using ase::containers::HashMap;
using ase::containers::HashSet;
using ase::containers::IntMixHash;
using ase::containers::Map;
using ase::containers::Pair;
using ase::containers::RingBuffer;
using ase::containers::Set;
using ase::containers::Size;
using ase::containers::Vector;

TEST_CASE("containers.hpp - index entities by composite key") {
    // Two 32-bit halves packed into one key, the form of an owner/slot index.
    const std::uint64_t key_a = (std::uint64_t{7} << 32) | 3u;
    const std::uint64_t key_b = (std::uint64_t{3} << 32) | 7u;

    SUBCASE("IntMixHash scatters keys that share their bits") {
        const IntMixHash hash{};
        CHECK(hash(key_a) != hash(key_b));
        CHECK(hash(key_a) == hash(key_a));
    }

    SUBCASE("HashMap finds each entry under its composite key") {
        HashMap<std::uint64_t, std::uint32_t, IntMixHash> index;
        index[key_a] = 11u;
        index[key_b] = 22u;
        REQUIRE(index.size() == 2u);
        CHECK(index.at(key_a) == 11u);
        CHECK(index.at(key_b) == 22u);
        CHECK(index.find((std::uint64_t{7} << 32) | 4u) == index.end());
    }

    SUBCASE("HashSet keeps a key once") {
        HashSet<std::uint64_t, IntMixHash> seen;
        CHECK(seen.insert(key_a).second);
        CHECK_FALSE(seen.insert(key_a).second);
        CHECK(seen.size() == 1u);
    }
}

TEST_CASE("containers.hpp - accumulate values per owner") {
    SUBCASE("Map sums per owner and walks the owners in key order") {
        Map<std::uint32_t, float> per_owner;
        per_owner[5u] += 1.5f;
        per_owner[2u] += 0.5f;
        per_owner[5u] += 2.0f;
        REQUIRE(per_owner.size() == 2u);
        CHECK(per_owner.begin()->first == 2u);
        CHECK(per_owner.at(5u) == doctest::Approx(3.5f));
    }

    SUBCASE("Set orders and deduplicates the owners") {
        Set<std::uint32_t> owners{9u, 1u, 9u, 4u};
        REQUIRE(owners.size() == 3u);
        CHECK(*owners.begin() == 1u);
        CHECK(*owners.rbegin() == 9u);
    }

    SUBCASE("Vector of Pair keeps owner and value together in insertion order") {
        Vector<Pair<std::uint32_t, float>> rows;
        rows.emplace_back(3u, 0.25f);
        rows.emplace_back(1u, 0.75f);
        REQUIRE(rows.size() == 2u);
        CHECK(rows.front().first == 3u);
        CHECK(rows.back().second == doctest::Approx(0.75f));
    }
}

TEST_CASE("containers.hpp - fixed capacity") {
    SUBCASE("Array is value-initialised and filled in place") {
        Array<std::uint32_t, 4> slots{};
        CHECK(slots.size() == 4u);
        CHECK(slots[0] == 0u);
        slots.fill(6u);
        std::uint32_t sum = 0u;
        for (const std::uint32_t value : slots) {
            sum += value;
        }
        CHECK(sum == 24u);
    }

    SUBCASE("RingBuffer hands values back in the order they came") {
        RingBuffer<std::uint32_t, 4> queue;
        CHECK(queue.push(1u));
        CHECK(queue.push(2u));
        std::uint32_t out = 0u;
        REQUIRE(queue.pop(out));
        CHECK(out == 1u);
        REQUIRE(queue.pop(out));
        CHECK(out == 2u);
        CHECK(queue.empty());
    }

    SUBCASE("Size counts a container's elements") {
        const Size count = static_cast<Size>(Vector<std::uint32_t>{1u, 2u, 3u}.size());
        CHECK(count == 3u);
    }
}
