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
 * @category    structure/container/sequential
 * @created     2026-01-07
 * @modified    2026-01-07
 * @version     1.0.0
 *
 * =============================================================================
 */

#include <ase/containers/types.hpp>
#include <atomic>
#include <cstddef>
#include <new>
#include <cassert>
// Kein <array> mehr (2026-08-22): die Datei hat NIE ein std::array benutzt — der Speicher ist
// ein rohes std::byte-Feld, und das kommt aus <cstddef>. Der Include stand hier als einziger
// Grund fuer einen CRITICAL-Befund ("std::array forbidden"), ohne eine einzige Benutzung.
// <new> bleibt: das placement new in emplace() braucht es, auch wenn clangd es als unbenutzt
// meldet — der Melder sieht den Operator nicht, nur den Typnamen.
// Kein <optional> mehr: pop() gibt seit dem 2026-08-22 bool zurueck und schreibt in ein
// Ausgabeziel. Der Header trug std::optional bis dahin bis in jeden Verbraucher — siehe pop().

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
        destroy_remaining();
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

    /// Emplace value in-place, 0 bis 3 Konstruktorargumente.
    /// @return true if emplaced, false if buffer full
    ///
    /// VIER UEBERLADUNGEN STATT EINES PARAMETERPAKETS (2026-08-22). Vorher stand hier
    /// `template<typename... Args> bool emplace(Args&&... args)`. Die Faehigkeit ist
    /// dieselbe geblieben: an Ort und Stelle konstruieren, OHNE Verschiebung und OHNE
    /// placement new an der Aufrufstelle. Nur das Paket ist weg.
    ///
    /// DER RESTPREIS, ausdruecklich, damit ihn der Naechste findet statt ihn neu zu
    /// entdecken: **vorher nahm emplace JEDE Aritaet, jetzt null bis drei.** Braucht ein T
    /// vier Konstruktorargumente, fehlt die Ueberladung — und das faellt beim Uebersetzen
    /// auf, nicht im Betrieb. Eine vierte anzuhaengen ist eine Zeile nach demselben Muster.
    /// GEMESSEN am 2026-08-22: die einzige emplace-Aufrufstelle im ganzen Baum ist
    /// `CHECK(buffer.emplace(1.0f, 2.0f))` in ring_buffer_test.cpp — ZWEI Argumente; die Drei
    /// ist bereits eine Reserve.
    ///
    /// Die Aufrufseite ist NICHT angefasst worden. `push(const T&)` und `push(T&&)` binden
    /// unveraendert an die einargumentige Form.
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

    // =========================================================================
    // Consumer Operations (call from consumer thread ONLY)
    // =========================================================================

    /// Pop value from buffer into `out`
    /// @param out receives the value; untouched when the buffer is empty
    /// @return true if a value was moved out, false if the buffer was empty
    ///
    /// DIESE FORM TRAEGT KEINE TYPENTSCHEIDUNG IN IHRE AUFRUFER, und das ist ihr ganzer Zweck
    /// (umgestellt 2026-08-22). Vorher lautete sie `std::optional<T> pop()` und zwang damit
    /// JEDEN Verbraucher zu std::optional. Der Validator zeigt dafuer auf ase::types::Option —
    /// inhaltlich zu Recht, denn es IST eine Kopplung.
    ///
    /// NUR WAERE DAS DIESELBE KRANKHEIT MIT ANDEREM ERREGER GEWESEN: `Option<T> pop()` haette
    /// jeden Verbraucher zu ase-types gezwungen und dazu die ERSTE ase::-Abhaengigkeit der
    /// ganzen Schicht gezogen. GEMESSEN am selben Tag: von den elf Modulen unter foundation/
    /// bindet KEIN EINZIGES ein ase::-Ziel (die vier mit target_link_libraries binden
    /// ausschliesslich Externe wie bsoncxx oder OpenSSL), und SECHS tragen den Satz woertlich
    /// in ihrer CMakeLists — "VERBOTEN: No dependencies allowed in Layer 0!", diese Datei
    /// eingeschlossen (CMakeLists.txt:112).
    ///
    /// `bool pop(T&)` loest die Kopplung auf, statt sie zu verschieben: es gibt nichts mehr,
    /// worauf die Regel zeigen koennte, und die Sperre bleibt unangetastet. Der Korpus hatte
    /// das bereits entschieden (PLAN_ASE_GEOVIS_PHASE_02_ROSTER.md, Korrektur 2026-08-15):
    /// "Dass beide Seiten einen Typ sehen muessten, ist kein Argument fuer L0: es beschreibt
    /// eine Kopplung, die WOANDERS AUFZULOESEN IST."
    ///
    /// DER PREIS, ausdruecklich benannt: `if (auto v = buf.pop())` traegt nicht mehr. Der
    /// Aufrufer haelt das Ziel selbst und fragt den Rueckgabewert:
    ///     T v;
    ///     while (buf.pop(v)) { handle(v); }
    /// GEMESSEN waren das zwei Dateien im ganzen Baum — dependency_sorter.cpp und der Test
    /// dieses Moduls. Bei zwei Dateien ist der Preis bezahlbar; bei zwanzig waere die
    /// Abwaegung eine andere gewesen, und das gehoert dazugesagt.
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

    // =========================================================================
    // Batch Operations (single-threaded only!)
    // =========================================================================

    /// Clear all elements (NOT thread-safe!)
    void clear() {
        destroy_remaining();
    }

private:
    static constexpr size_t MASK = Capacity - 1;

    /// Ein Speicherplatz. Die Union beginnt die Lebenszeit ihres Members NICHT — dafuer
    /// stehen der leere Konstruktor und der leere Destruktor da, und nur deshalb darf ein
    /// nicht-trivialer Typ hier liegen, ohne konstruiert zu werden.
    ///
    /// DAS IST DIE STELLE, AN DER AM 2026-08-22 ZWEI `reinterpret_cast` VERSCHWUNDEN SIND —
    /// und zwar, weil die UMDEUTUNG verschwunden ist, nicht weil das Muster anders
    /// geschrieben wird. Vorher lag hier ein `alignas(T) std::byte`-Feld, das an zwei
    /// Stellen als T gelesen wurde; jetzt ist der Speicher TYPISIERT, und `&slot.value` ist
    /// ein `T*` ohne jeden Cast. Es gibt nichts mehr, worauf die Regel zeigen koennte.
    ///
    /// WAS SICH DABEI NICHT AENDERT, namentlich: Kapazitaet (Capacity Plaetze),
    /// Speicherbedarf (`sizeof(Slot) == sizeof(T)`), Ausrichtung (die Union erbt
    /// `alignof(T)`), die Lebenszeitverwaltung (placement new und Destruktoraufruf wie
    /// zuvor) und die Zusage, dass T KEINEN Default-Konstruktor braucht — die prueft
    /// `ring_buffer_test.cpp` mit seinem `MoveOnly`, das nur `MoveOnly(int)` hat.
    ///
    /// Kopieren und Verschieben bleiben geloescht. Die Union macht sie fuer einen
    /// nicht-trivialen Member ohnehin geloescht; die Klasse hatte sie schon vorher
    /// ausdruecklich entfernt, also faellt das zusammen und nichts geht verloren.
    union Slot {
        Slot() noexcept {}   // beginnt keine Lebenszeit
        ~Slot() {}           // beendet keine
        T value;
    };

    /// Adresse des Platzes `index`, in den Bereich gefaltet.
    ///
    /// Am 2026-08-22 aus VIER gleichlautenden Zeilen zusammengezogen (emplace, pop, peek,
    /// destroy_remaining). Vorher stand dieselbe Adressrechnung viermal da; jede Aenderung
    /// an ihr haette an vier Stellen stimmen muessen.
    ///
    /// Zwei Ueberladungen und nicht eine: peek() ist const. Ein `const_cast` haette die
    /// zweite gespart und dafuer einen Cast eingefuehrt, den es jetzt nirgends mehr gibt.
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
    /// in dem noch kein Element steht.
    ///
    /// Getrennt sind sie, seit emplace() am 2026-08-22 in vier Aritaeten zerfiel: sonst
    /// stuende diese Belegung viermal da, und die vier Fassungen koennten auseinanderlaufen.
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
    /// Called by the destructor and by clear(). Both said `while (pop()) {}` until
    /// 2026-08-22 and kept saying it after pop() had grown its out-parameter — the header
    /// stayed green the whole time, because a template member is only checked when it is
    /// INSTANTIATED, and the destructor is instantiated in the CONSUMER. The error surfaced
    /// in a foreign file (core/ase-ecs/src/internal/dependency_sorter.cpp) pointing at a
    /// line that was never wrong.
    ///
    /// NOT written as `T scratch{}; while (pop(scratch)) {}`: that form would quietly make
    /// default-constructibility a requirement of T, and it would bind just as invisibly -
    /// only where a destructor gets instantiated. The counter-example already exists in
    /// this module's own test: ring_buffer_test.cpp gives MoveOnly a single MoveOnly(int)
    /// and no default constructor. Walking the indices asks nothing of T that emplace() did
    /// not already ask.
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
