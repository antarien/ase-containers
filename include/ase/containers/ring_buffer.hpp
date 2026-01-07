#pragma once

/**
 * =============================================================================
 * ASE CONTAINERS - RING BUFFER
 * =============================================================================
 *
 * @file        ring_buffer.hpp
 * @brief       RingBuffer - Lock-free SPSC circular buffer
 * @description Single-Producer Single-Consumer lock-free circular buffer.
 *              Ideal for producer-consumer patterns between threads.
 *
 *              Memory Layout:
 *                [0][1][2][3][4][5][6][7]  (fixed capacity, power of 2)
 *                 ^           ^
 *                head        tail
 *
 *              Lock-free via atomic head/tail with acquire-release semantics.
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

#include <ase/containers/types.hpp>
#include <array>
#include <atomic>
#include <optional>
#include <new>
#include <cassert>

namespace ase::containers {

namespace detail {

/// Check if value is power of 2
constexpr bool is_power_of_two(size_t n) noexcept {
    return n > 0 && (n & (n - 1)) == 0;
}

/// Round up to next power of 2
constexpr size_t next_power_of_two(size_t n) noexcept {
    if (n == 0) return 1;
    --n;
    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    n |= n >> 8;
    n |= n >> 16;
    n |= n >> 32;
    return ++n;
}

}  // namespace detail

/**
 * RingBuffer - Lock-free Single-Producer Single-Consumer circular buffer
 *
 * @tparam T Value type to store
 * @tparam Capacity Buffer capacity (must be power of 2)
 *
 * Usage:
 *   RingBuffer<Event, 1024> events;
 *
 *   // Producer thread
 *   events.push(Event{...});
 *
 *   // Consumer thread
 *   if (auto event = events.pop()) {
 *       handle(*event);
 *   }
 *
 * Thread Safety:
 *   - Single producer, single consumer ONLY
 *   - push() called from producer thread only
 *   - pop() called from consumer thread only
 *   - No external synchronization needed
 */
template<typename T, size_t Capacity = RING_BUFFER_DEFAULT_CAPACITY>
class RingBuffer {
    static_assert(detail::is_power_of_two(Capacity),
                  "RingBuffer capacity must be power of 2");
    static_assert(Capacity <= RING_BUFFER_MAX_CAPACITY,
                  "RingBuffer capacity exceeds maximum");

public:
    // =========================================================================
    // Types
    // =========================================================================

    using value_type = T;
    using size_type = Size;

    // =========================================================================
    // Construction
    // =========================================================================

    RingBuffer() noexcept : head_(0), tail_(0) {}

    // Non-copyable, non-movable (due to atomics)
    RingBuffer(const RingBuffer&) = delete;
    RingBuffer& operator=(const RingBuffer&) = delete;
    RingBuffer(RingBuffer&&) = delete;
    RingBuffer& operator=(RingBuffer&&) = delete;

    ~RingBuffer() {
        // Destroy any remaining elements
        while (pop()) {}
    }

    // =========================================================================
    // Capacity
    // =========================================================================

    /// Maximum number of elements that can be stored
    [[nodiscard]] static constexpr size_t capacity() noexcept {
        return Capacity;
    }

    /// Current number of elements (approximate, may be stale)
    [[nodiscard]] size_type size() const noexcept {
        size_t head = head_.load(std::memory_order_acquire);
        size_t tail = tail_.load(std::memory_order_acquire);
        return static_cast<size_type>((tail - head) & MASK);
    }

    /// Check if empty (may be stale in multi-threaded context)
    [[nodiscard]] bool empty() const noexcept {
        return head_.load(std::memory_order_acquire) ==
               tail_.load(std::memory_order_acquire);
    }

    /// Check if full (may be stale in multi-threaded context)
    [[nodiscard]] bool full() const noexcept {
        size_t tail = tail_.load(std::memory_order_acquire);
        size_t head = head_.load(std::memory_order_acquire);
        return ((tail + 1) & MASK) == (head & MASK);
    }

    // =========================================================================
    // Producer Operations (call from producer thread ONLY)
    // =========================================================================

    /// Push value to buffer
    /// @return true if pushed, false if buffer full
    bool push(const T& value) {
        return emplace(value);
    }

    /// Push value to buffer (move)
    /// @return true if pushed, false if buffer full
    bool push(T&& value) {
        return emplace(std::move(value));
    }

    /// Emplace value in-place
    /// @return true if emplaced, false if buffer full
    template<typename... Args>
    bool emplace(Args&&... args) {
        size_t tail = tail_.load(std::memory_order_relaxed);
        size_t next_tail = (tail + 1) & MASK;

        // Check if full
        if (next_tail == head_.load(std::memory_order_acquire)) {
            return false;  // Buffer full
        }

        // Construct element in-place
        T* slot = reinterpret_cast<T*>(buffer_) + (tail & MASK);
        new (slot) T{std::forward<Args>(args)...};

        // Publish to consumer
        tail_.store(next_tail, std::memory_order_release);
        return true;
    }

    // =========================================================================
    // Consumer Operations (call from consumer thread ONLY)
    // =========================================================================

    /// Pop value from buffer
    /// @return Value if available, nullopt if empty
    [[nodiscard]] std::optional<T> pop() {
        size_t head = head_.load(std::memory_order_relaxed);

        // Check if empty
        if (head == tail_.load(std::memory_order_acquire)) {
            return std::nullopt;  // Buffer empty
        }

        // Read element
        T* ptr = reinterpret_cast<T*>(buffer_) + (head & MASK);
        T value = std::move(*ptr);
        ptr->~T();

        // Publish to producer
        head_.store((head + 1) & MASK, std::memory_order_release);
        return value;
    }

    /// Peek at front element without removing
    /// @return Pointer to front element, nullptr if empty
    [[nodiscard]] const T* peek() const noexcept {
        size_t head = head_.load(std::memory_order_relaxed);
        if (head == tail_.load(std::memory_order_acquire)) {
            return nullptr;
        }
        return reinterpret_cast<const T*>(buffer_) + (head & MASK);
    }

    // =========================================================================
    // Batch Operations (single-threaded only!)
    // =========================================================================

    /// Clear all elements (NOT thread-safe!)
    void clear() {
        while (pop()) {}
    }

private:
    static constexpr size_t MASK = Capacity - 1;

    // Aligned storage for elements
    alignas(T) std::byte buffer_[Capacity * sizeof(T)];

    // Cache line padding to prevent false sharing
    alignas(64) std::atomic<size_t> head_;
    alignas(64) std::atomic<size_t> tail_;
};

}  // namespace ase::containers
