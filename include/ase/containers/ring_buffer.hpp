#pragma once

/**
 * ASE CORE INFRASTRUCTURE HEADER
 *
 * @file        ring_buffer.hpp
 * @design      DSGN_016
 * @brief       RingBuffer - Lock-free SPSC circular buffer
 * @description Single-Producer Single-Consumer lock-free circular buffer.
 *              Ideal for producer-consumer patterns between threads. Its user in the
 *              tree is the FIFO of the dependency sort (core/ase-ecs,
 *              src/internal/dependency_sorter.cpp), which orders the systems of a
 *              schedule.
 *
 *              Memory Layout:
 *                [0][1][2][3][4][5][6][7]  (fixed capacity, power of 2)
 *                 ^           ^
 *                head        tail
 *
 *              Lock-free via atomic head/tail with acquire-release semantics.
 *
 *              Includes: the storage is a typed union array, so no <array> is needed;
 *              <new> stays for the placement new in emplace(). There is no <optional>:
 *              pop() fills an output target and answers with bool.
 *
 * @module      ase-containers
 * @layer       0 (Foundation)
 * @category    structure/container/sequential
 * @usecase     usecase:100033 - order systems by dependency
 * @actor       Entity Registry Steward (ENTIREGSTE)
 * @created     2026-01-07
 * @modified    2026-10-30
 * @version     1.0.1
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

#include <ase/containers/types.hpp>
#include <atomic>
#include <cstddef>
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
 *   // Consumer thread - the caller holds the target, pop() reports whether it filled it
 *   Event event;
 *   while (events.pop(event)) {
 *       handle(event);
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
    // Types

    using value_type = T;
    using size_type = Size;

    // Construction

    RingBuffer() noexcept : head_(0), tail_(0) {}

    // Not constructible or assignable from another instance (due to atomics)
    RingBuffer(const RingBuffer&) = delete;
    RingBuffer& operator=(const RingBuffer&) = delete;
    RingBuffer(RingBuffer&&) = delete;
    RingBuffer& operator=(RingBuffer&&) = delete;

    ~RingBuffer() {
        destroy_remaining();
    }

    // Capacity

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

    // Producer Operations (call from producer thread ONLY)

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

    /// Emplace value in-place, 0 bis 3 Konstruktorargumente.
    /// @return true if emplaced, false if buffer full
    ///
    /// VIER UEBERLADUNGEN STATT EINES PARAMETERPAKETS: jede konstruiert an Ort und Stelle,
    /// ohne Verschiebung und ohne placement new an der Aufrufstelle.
    ///
    /// DIE GRENZE, ausdruecklich: emplace nimmt null bis drei Argumente. Braucht ein T vier
    /// Konstruktorargumente, fehlt die Ueberladung - und das faellt beim Uebersetzen auf,
    /// nicht im Betrieb. Eine vierte anzuhaengen ist eine Zeile nach demselben Muster.
    ///
    /// `push(const T&)` und `push(T&&)` binden an die einargumentige Form.
    bool emplace() {
        size_t next_tail = 0;
        T* slot = this->try_reserve(next_tail);
        if (slot == nullptr) return false;
        new (slot) T{};
        this->publish(next_tail);
        return true;
    }

    template<typename A0>
    bool emplace(A0&& a0) {
        size_t next_tail = 0;
        T* slot = this->try_reserve(next_tail);
        if (slot == nullptr) return false;
        new (slot) T{std::forward<A0>(a0)};
        this->publish(next_tail);
        return true;
    }

    template<typename A0, typename A1>
    bool emplace(A0&& a0, A1&& a1) {
        size_t next_tail = 0;
        T* slot = this->try_reserve(next_tail);
        if (slot == nullptr) return false;
        new (slot) T{std::forward<A0>(a0), std::forward<A1>(a1)};
        this->publish(next_tail);
        return true;
    }

    template<typename A0, typename A1, typename A2>
    bool emplace(A0&& a0, A1&& a1, A2&& a2) {
        size_t next_tail = 0;
        T* slot = this->try_reserve(next_tail);
        if (slot == nullptr) return false;
        new (slot) T{std::forward<A0>(a0), std::forward<A1>(a1), std::forward<A2>(a2)};
        this->publish(next_tail);
        return true;
    }

    // Consumer Operations (call from consumer thread ONLY)

    /// Pop value from buffer into `out`
    /// @param out receives the value; untouched when the buffer is empty
    /// @return true if a value was moved out, false if the buffer was empty
    ///
    /// DIESE FORM TRAEGT KEINE TYPENTSCHEIDUNG IN IHRE AUFRUFER, und das ist ihr ganzer Zweck.
    /// Eine Rueckgabe als Option-Typ zwaenge jeden Verbraucher zu diesem Typ und zoege die
    /// erste ase::-Abhaengigkeit in Schicht 0, deren CMakeLists jede Abhaengigkeit verbietet.
    /// `bool pop(T&)` loest diese Kopplung auf, statt sie zu verschieben (so entschieden in
    /// PLAN_ASE_GEOVIS_PHASE_02_ROSTER.md: "Dass beide Seiten einen Typ sehen muessten, ist
    /// kein Argument fuer L0: es beschreibt eine Kopplung, die WOANDERS AUFZULOESEN IST.")
    ///
    /// Der Aufrufer haelt das Ziel selbst und fragt den Rueckgabewert:
    ///     T v;
    ///     while (buf.pop(v)) { handle(v); }
    [[nodiscard]] bool pop(T& out) {
        size_t head = head_.load(std::memory_order_relaxed);

        // Check if empty
        if (head == tail_.load(std::memory_order_acquire)) {
            return false;  // Buffer empty - `out` stays untouched
        }

        // Read element
        T* ptr = this->slot_at(head);
        out = std::move(*ptr);
        ptr->~T();

        // Publish to producer
        head_.store((head + 1) & MASK, std::memory_order_release);
        return true;
    }

    /// Peek at front element without removing
    /// @return Pointer to front element, nullptr if empty
    [[nodiscard]] const T* peek() const noexcept {
        size_t head = head_.load(std::memory_order_relaxed);
        if (head == tail_.load(std::memory_order_acquire)) {
            return nullptr;
        }
        return this->slot_at(head);
    }

    // Batch Operations (single-threaded only!)

    /// Clear all elements (NOT thread-safe!)
    void clear() {
        destroy_remaining();
    }

private:
    static constexpr size_t MASK = Capacity - 1;

    /// Ein Speicherplatz. Die Union beginnt die Lebenszeit ihres Members NICHT - dafuer
    /// stehen der leere Konstruktor und der leere Destruktor da, und nur deshalb darf ein
    /// nicht-trivialer Typ hier liegen, ohne konstruiert zu werden.
    ///
    /// Der Speicher ist TYPISIERT: `&slot.value` ist ein `T*` ohne jeden Cast, also gibt es
    /// keine Umdeutung roher Bytes. Kapazitaet (Capacity Plaetze), Speicherbedarf
    /// (`sizeof(Slot) == sizeof(T)`) und Ausrichtung (die Union erbt `alignof(T)`) sind die
    /// eines rohen Feldes; die Lebenszeit verwalten placement new und der Destruktoraufruf,
    /// und T braucht KEINEN Default-Konstruktor - das prueft `ring_buffer_test.cpp` mit
    /// seinem `MoveOnly`, das nur `MoveOnly(int)` hat.
    ///
    /// Konstruktion und Zuweisung aus einer anderen Instanz sind geloescht: die Union
    /// loescht sie fuer einen nicht-trivialen Member ohnehin, und die Klasse entfernt sie
    /// ausdruecklich.
    union Slot {
        Slot() noexcept {}   // beginnt keine Lebenszeit
        ~Slot() {}           // beendet keine
        T value;
    };

    /// Adresse des Platzes `index`, in den Bereich gefaltet - die eine Adressrechnung, die
    /// emplace, pop, peek und destroy_remaining teilen.
    ///
    /// Zwei Ueberladungen und nicht eine: peek() ist const. Ein `const_cast` haette die
    /// zweite gespart und dafuer einen Cast eingefuehrt.
    [[nodiscard]] T* slot_at(size_t index) noexcept {
        return &buffer_[index & MASK].value;
    }

    [[nodiscard]] const T* slot_at(size_t index) const noexcept {
        return &buffer_[index & MASK].value;
    }

    /// Platz fuer das naechste Element belegen, oder nullptr wenn der Puffer voll ist.
    ///
    /// ZUSAMMEN MIT publish() BILDET DAS DIE ZWEI HAELFTEN EINES SCHREIBVORGANGS, und die
    /// Reihenfolge ist Vertrag, nicht Stil: erst belegen, DANN konstruieren, DANN
    /// veroeffentlichen. Wer frueher veroeffentlicht, gibt dem Verbraucher einen Platz frei,
    /// in dem noch kein Element steht. Die vier Aritaeten von emplace() teilen diese eine
    /// Belegung, damit sie nicht auseinanderlaufen koennen.
    [[nodiscard]] T* try_reserve(size_t& next_tail_out) noexcept {
        const size_t tail = tail_.load(std::memory_order_relaxed);
        next_tail_out = (tail + 1) & MASK;
        if (next_tail_out == head_.load(std::memory_order_acquire)) {
            return nullptr;  // Buffer full
        }
        return this->slot_at(tail);
    }

    /// Den belegten Platz fuer den Verbraucher freigeben. NUR nach der Konstruktion rufen.
    void publish(size_t next_tail) noexcept {
        tail_.store(next_tail, std::memory_order_release);
    }

    /// Destroy every element still stored, then mark the buffer empty.
    ///
    /// Called by the destructor and by clear(). It walks the indices instead of popping:
    /// a template member is only checked when it is INSTANTIATED, and the destructor is
    /// instantiated in the CONSUMER, so a mismatch with pop() surfaces in a foreign file.
    ///
    /// NOT written as `T scratch{}; while (pop(scratch)) {}`: that form would quietly make
    /// default-constructibility a requirement of T, and it would bind just as invisibly -
    /// only where a destructor gets instantiated. The counter-example exists in this
    /// module's own test: ring_buffer_test.cpp gives MoveOnly a single MoveOnly(int) and no
    /// default constructor. Walking the indices asks nothing of T that emplace() does not
    /// already ask.
    void destroy_remaining() {
        size_t head = head_.load(std::memory_order_relaxed);
        const size_t tail = tail_.load(std::memory_order_acquire);
        while (head != tail) {
            this->slot_at(head)->~T();
            head = (head + 1) & MASK;
        }
        head_.store(tail, std::memory_order_release);
    }

    // Speicher fuer die Elemente. Typisiert, nicht roh: siehe Slot oben.
    Slot buffer_[Capacity];

    // Cache line padding to prevent false sharing
    alignas(64) std::atomic<size_t> head_;
    alignas(64) std::atomic<size_t> tail_;
};

}  // namespace ase::containers
