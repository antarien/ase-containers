/**
 * ASE CONTAINERS - RingBuffer Unit Tests
 *
 * @file        ring_buffer_test.cpp
 * @brief       Cases for the fixed-capacity ring buffer, single- and multi-threaded.
 * @description Covers the basic operations of the fixed-capacity ring buffer and its behaviour at
 *              both of its edges — the full buffer and the empty one.
 *
 *              DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN is set by the build (tests/CMakeLists.txt,
 *              target_compile_definitions on the target ase-containers-test) instead of here as a
 *              #define. The define is a property of the TARGET: it decides which translation unit
 *              receives main(), and this target has exactly one. Whoever reads this file looking
 *              for main() finds it there.
 *
 * @module      ase-containers
 * @layer       0 (Foundation)
 * @category    structure/container/sequential
 * @usecase     usecase:101396 - assure engine component quality
 * @actor       Engine Unit Test (ENGUTEST)
 * @created     2026-01-07
 * @modified    2026-11-05
 * @version     1.0.1
 */

#include <doctest/doctest.h>
#include <ase/containers/ring_buffer.hpp>
#include <thread>
#include <atomic>

using namespace ase::containers;

TEST_CASE("RingBuffer - Basic Operations") {
    RingBuffer<int, 8> buffer;

    SUBCASE("Empty buffer") {
        CHECK(buffer.empty());
        CHECK(buffer.size() == 0);
        CHECK(buffer.capacity() == 8);
        CHECK_FALSE(buffer.full());
    }

    SUBCASE("Push and pop single element") {
        CHECK(buffer.push(42));
        CHECK_FALSE(buffer.empty());
        CHECK(buffer.size() == 1);

        // pop() writes into the target and reports whether it did (changed 2026-08-20, before that
        // std::optional). The test holds the target itself.
        int val = 0;
        CHECK(buffer.pop(val));
        CHECK(val == 42);
        CHECK(buffer.empty());
    }

    SUBCASE("Push multiple elements") {
        for (int i = 0; i < 7; ++i) {
            CHECK(buffer.push(i * 10));
        }
        CHECK(buffer.size() == 7);

        // Buffer should be full now (capacity-1 elements usable)
        CHECK(buffer.full());
        CHECK_FALSE(buffer.push(999));
    }

    SUBCASE("FIFO order") {
        buffer.push(1);
        buffer.push(2);
        buffer.push(3);

        int val = 0;
        CHECK(buffer.pop(val));  CHECK(val == 1);
        CHECK(buffer.pop(val));  CHECK(val == 2);
        CHECK(buffer.pop(val));  CHECK(val == 3);
    }

    SUBCASE("Pop from empty buffer") {
        // The target stays UNTOUCHED when there is nothing to pop - that is the promise of the
        // signature, and it is checked here along with the return value.
        int val = -1;
        CHECK_FALSE(buffer.pop(val));
        CHECK(val == -1);
    }
}

TEST_CASE("RingBuffer - Peek") {
    RingBuffer<int, 8> buffer;

    SUBCASE("Peek empty buffer") {
        CHECK(buffer.peek() == nullptr);
    }

    SUBCASE("Peek does not remove") {
        buffer.push(42);
        CHECK(buffer.peek() != nullptr);
        CHECK(*buffer.peek() == 42);
        CHECK(buffer.size() == 1);  // Still there

        // Multiple peeks
        CHECK(*buffer.peek() == 42);
        CHECK(*buffer.peek() == 42);
        CHECK(buffer.size() == 1);
    }
}

TEST_CASE("RingBuffer - Wraparound") {
    RingBuffer<int, 8> buffer;

    // Fill and partially drain
    for (int i = 0; i < 5; ++i) {
        buffer.push(i);
    }
    for (int i = 0; i < 3; ++i) {
        int discard = 0;
        (void)buffer.pop(discard);  // the value is dropped on purpose - the test measures the wraparound
    }

    // Add more (causes wraparound)
    for (int i = 5; i < 10; ++i) {
        buffer.push(i);
    }

    // Check FIFO order across wraparound
    for (int expected = 3; expected < 10; ++expected) {
        int val = 0;
        CHECK(buffer.pop(val));
        CHECK(val == expected);
    }
}

TEST_CASE("RingBuffer - Clear") {
    RingBuffer<int, 8> buffer;
    buffer.push(1);
    buffer.push(2);
    buffer.push(3);

    buffer.clear();
    CHECK(buffer.empty());
    CHECK(buffer.size() == 0);
}

TEST_CASE("RingBuffer - Emplace") {
    struct Point {
        float x, y;
    };

    RingBuffer<Point, 8> buffer;
    CHECK(buffer.emplace(1.0f, 2.0f));

    Point val{0.0f, 0.0f};
    CHECK(buffer.pop(val));
    CHECK(val.x == 1.0f);
    CHECK(val.y == 2.0f);
}

TEST_CASE("RingBuffer - Move semantics") {
    struct MoveOnly {
        int value;
        MoveOnly(int v) : value(v) {}
        MoveOnly(const MoveOnly&) = delete;
        MoveOnly& operator=(const MoveOnly&) = delete;
        MoveOnly(MoveOnly&& other) noexcept : value(other.value) { other.value = 0; }
        MoveOnly& operator=(MoveOnly&& other) noexcept {
            value = other.value;
            other.value = 0;
            return *this;
        }
    };

    RingBuffer<MoveOnly, 8> buffer;
    buffer.push(MoveOnly{42});

    // MoveOnly has a move assignment operator - exactly what the signature needs, because pop()
    // assigns INTO the target instead of returning a value. The case stands here on purpose: it is
    // the only one that makes this requirement on T visible.
    MoveOnly val{0};
    CHECK(buffer.pop(val));
    CHECK(val.value == 42);
}

TEST_CASE("RingBuffer - SPSC Thread Safety") {
    constexpr int NUM_ITEMS = 10000;
    RingBuffer<int, 1024> buffer;
    std::atomic<bool> done{false};
    std::atomic<int> consumed{0};

    // Producer thread
    std::thread producer([&]() {
        for (int i = 0; i < NUM_ITEMS; ++i) {
            while (!buffer.push(i)) {
                // Spin until space available
                std::this_thread::yield();
            }
        }
        done = true;
    });

    // Consumer thread
    std::thread consumer([&]() {
        int expected = 0;
        while (!done || !buffer.empty()) {
            int val = 0;
            if (buffer.pop(val)) {
                CHECK(val == expected);
                ++expected;
                ++consumed;
            } else {
                std::this_thread::yield();
            }
        }
    });

    producer.join();
    consumer.join();

    CHECK(consumed == NUM_ITEMS);
    CHECK(buffer.empty());
}
