# Corium Memory Manifesto
> Im lazy, so all subsytems get the copy

## Why We Manage Memory This Way
Memory is the most common source of performance death and "impossible" bugs. In Spectra, we don't trust the OS or the language to handle memory for us. We own every byte, we track every allocation, and we align everything to the hardware. 

## 1. The CRT Heap: Stay Away
We avoid the standard Windows CRT Heap (`new` and `delete`) whenever possible.
*   **The "Why":** The Windows CRT heap is heavy, bulky, and unpredictable. For a high-performance renderer, we need to own every memory call and manage the lifecycle ourselves.
*   **Object Construction:** Use placement new with allocators. Allocators only provide raw memory; they don't construct objects.
*   **The Reality:** Raw `new` and `delete` are allowed *only* in the early stages of a component. Once the infrastructure is ready, everything must move to specialized allocators.

### Allocator Usage Example
```cpp
// Object construction via placement new
T* p_Object = new (allocator.allocate<T>()) T(args...);

// Destruction (Absolute Control)
p_Object->~T(); 
allocator.free(p_Object);
```

## 2. SMT & False Sharing: Align to 64
We enforce a scorched-earth policy on alignment. Everything significant is `alignas(64)`.
*   **The "Why":** Simultaneous Multithreading (SMT) is great until you hit false sharing. I don't want to spend three weeks debugging why a thread-safe variable is slow because it's bouncing between cache lines. We align to 64 bytes to ensure that different threads stay in their own lanes.

### Alignment & Padding Example
```cpp
struct alignas(64) HotData {
    // Performance-critical members at the top
    float transform[16]; 
    uint32_t id;
    
    // Padding to ensure gapless structs and prevent false sharing
    uint8_t padding[12]; 
};
static_assert(sizeof(HotData) == 64, "Alignment mismatch!");
```

## 3. Destruction: Absolute Control
We don't use `delete`. We use manual destructor calls followed by an allocator free.
*   **The "Why":** Absolute control is the goal. I want to know exactly when an object dies and when its memory is actually returned to the pool.
*   **RAII:** Still use RAII for heap-resident resources to ensure cleanup on early returns or failures, but the underlying memory must be allocator-backed.

## 4. Cache Awareness: The Size Rule
Every struct must start with a comment indicating its size (e.g., `// Size: 64B`).
*   **The "Why":** This is a personal touch to keep a constant eye on cache-line headroom. I want to know exactly how much space is left in a cache line so I can see if the struct is "extendable" later without blowing the budget.

### Cache-Aware Layout
*   **L1 Hot Path:** ≤64B
*   **L2 Medium Path:** ≤128B
*   **Larger Blobs:** Use pointers to external memory; avoid virtual methods or internal heap allocations in hot structs.

## 5. Threading: Lock-Free Arenas
We use `ThreadLocalAllocator` or `TaskFrameArena` for the hot path.
*   **The "Why":** Lock contention is a death spiral for a renderer. By giving every thread its own arena, we keep the hot path entirely lock-free.
*   **Pattern:** Bind memory during task creation to avoid hidden allocator access.

### Thread-Local Pattern Example
```cpp
auto l_task = [arena = tlsArena]() {
    // Use arena safely without locks
    auto p_temp = arena.allocate<TempObject>();
};
```

## 6. Passing & Ownership Discipline
*   **Trivial Types:** Pass by value (ints, enums, POD structs).
*   **Heavy Objects:** Pass by const reference or `ro_` (Reference to Object).
*   **Optional Work:** Use pointers (`p_`) with `nullptr` checks to signal "maybe-exists" semantics.
*   **Forwarding:** Use universal references (`T&&`) only in template or lambda forwarding contexts.

## The Golden Rule
If an allocator or an alignment rule is causing a bottleneck in a unique edge case—**fix the allocator, don't bypass the rule**. We keep the standards so we can keep our sanity.
