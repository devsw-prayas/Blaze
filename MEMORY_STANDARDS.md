# Corium Memory & Ownership Standards

## Motivation
Corium enforces strict performance and memory correctness constraints, defining how memory is allocated, passed, cleaned up, and structured to ensure high throughput, cache-friendliness, and debug-ability, where every byte, bounce, and allocation matters.

## A) Allocator Rules
1. Raw `new` and `delete` are banned for bypassing sanitation, tracking, and instrumentation tools, requiring all memory to be acquired via allocators like `ArenaAllocator` or `PlatformPageAllocator`.
2. Object construction must use placement new (`T* p_Object = new (allocator.allocate<T>()) T(args...);`) since allocators only provide raw memory and do not construct objects.
3. Platform-specific allocators interfacing with OS APIs must be prefixed with `Platform` (e.g., `PlatformVirtualAllocator`, `PlatformAlignedHeap`) to ensure architectural clarity.
4. Heap allocations are forbidden in hot paths, requiring up-front allocation, buffer reuse, object pooling, or stack memory to avoid performance hits.
5. All allocators must expose instrumentation hooks (allocation tracker, leak auditor, profiler sampler) to integrate with tools like Kerbecs for profiling and debugging.

## B) RAII Enforcement
1. RAII is mandatory for heap-resident resources, ensuring cleanup via destructors for exception safety and scope-based guarantees.
2. Stack memory must avoid lingering references beyond scope and overloading stack frames, especially in deep task graphs, to prevent bloat and errors.
3. Pools or arenas must be wrapped in RAII-enabled wrappers to ensure no leaks occur on early returns, failure paths, or panics.

## C) Ownership Contracts
1. Ownership must be explicit, with functions returning ownership using `unique_ptr<T>` or clearly documenting raw pointer ownership.
2. Functions accepting ownership must delete or store the object, ensuring no ambiguity in responsibility.
3. Use `unique_ptr` for sole responsibility and `shared_ptr` only for shared ownership with unknown destruction timing, avoiding unnecessary use.
4. Copy constructors for heavy types must be opt-in to prevent unintended duplication of large objects.

## D) Reference Passing Discipline
1. Pass trivial types (ints, enums, POD structs) by value since they are cheap and safe.
2. Pass heavy objects (strings, containers, large structs) by const reference to avoid costly copying.
3. Use pointers with `nullptr` to signal optional work, not as a substitute for reference semantics, but for maybe-exists semantics.
4. Pass by universal reference (`T&&`) only in template or lambda forwarding contexts, ensuring safety and utility with perfect forwarding.
5. Explicitly name parameters with prefixes (e.g., `r_`, `p_`, `u_`, `su_`) to visually infer semantics and improve code readability.

## E) Destruction and Cleanup
1. Allocator-constructed objects must be explicitly destroyed using `allocator.destroy(p_Object)` or by manually calling the destructor (`p_Object->~T(); allocator.free(p_Object);`).
2. Allocators free bulk memory but do not destruct objects, leaving object destruction as the programmer’s responsibility unless using pooled handles or wrappers.
3. All memory must be freed in the same manner it was acquired to avoid orphaned pointers and mismatched frees.
4. Allocators should be trivially destructible unless managing pooled state or virtual memory regions, in which case they must be wrapped in a manager.

## F) Thread-Local Memory Patterns
1. Use thread-local allocators like `ThreadLocalAllocator` or `TaskFrameArena`, typically implemented with ring buffers or fixed-size pages, to prevent contention.
2. Avoid leaking shared memory into lambdas or thread workers, instead binding memory during lambda creation (e.g., `auto task = [arena = tlsArena]() { /* use arena safely */ };`).
3. Pass allocator references explicitly in threads to avoid hidden allocator access and maintain clarity.
4. Thread-local allocators must support debugging (bytes used, peaks, allocations), tagging (per-job, per-system), and sanity check tracing (e.g., TLS corruption detection).

## G) Alignment & Cache-Aware Structs
1. All data must be stored in aligned memory, targeting at least `alignas(32)` for AVX2 or `alignas(64)` for AVX-512 support.
2. Every struct must include a leading comment indicating total size (e.g., `// Size: 64 bytes (aligned to cache line)`) to eliminate guesswork.
3. Performance-critical members (SIMD/float/vector arrays, IDs, transforms) must be placed at the top of structs to enable fake-SoA patterns using AoS layouts.
4. Padding is required to ensure gapless structs and prevent false sharing, with `static_assert(sizeof(...) == expectedSize)` to lock in the size.
5. Cache-aware layout guidelines dictate ≤64B for L1 hot paths, ≤128B for L2 medium-critical paths, and pointers to external blobs for anything larger, avoiding virtual methods, internal heap allocations, or recursive containers in hot structs.