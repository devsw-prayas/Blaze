# Blaze

**Blaze** is a high-performance, modular multithreading engine for C++. It provides precise control over task orchestration, thread scheduling, and parallel execution — built from the ground up with a zero-overhead mindset. Blaze powers the concurrency layer of Spectra, enabling systems that demand deterministic behavior, fine-grained parallelism, and full control over memory and execution lifecycles.

---

## Capabilities

* **Thread Pooling Architecture**
  Provides multiple thread pool models including `DefaultThreadPool`, `CachedThreadPool`, `DefaultScheduledThreadPool`, and `CachedScheduledThreadPool`. Designed to balance core utilization, throughput, and responsiveness.

* **Work Stealing Execution**
  Decentralized task redistribution using `DefaultWorkStealerPool` and the `IWorkStealLoad` interface. Each worker owns a bounded queue with localized scheduling, enabling low-contention and high-throughput dynamic balancing.

* **SIMD-Friendly Bulk Execution**
  Supports batch-oriented, core-pinned execution via `ATaskEngine`, designed for high-density task dispatch. Operates under a shared allocator context with deterministic memory and cache behavior.

* **Composable Execution Chains**
  Functional-style composition through `IComposer`, `Composable`, and `Compose` modes. Enables declarative task pipelines (e.g. transform → intermediate → reduce), schedulable across pool or batch layers.

* **TLS-Based Execution Contexts**
  Execution metadata is accessible through static `thread_local` bindings:

  * `this_thread` → bound to current `IHandle*` (e.g. pool executor)
  * `this_worker` → active work stealing context
  * `this_core` → current bulk task engine (`ATaskEngine`)
  * `this_composition` → current composer context (`IComposer*`)

  Each exposes allocator bindings, state metadata, and optional profiling hooks.

* **Instrumentation Hooks**
  All interfaces expose optional `void*` instrumentation hooks to integrate tracing or logging without overhead when unused. Fully compatible with Stratum for timeline/event stream capture.

* **Modular & Cross-Boundary Safe**
  Blaze is designed with strict separation of interfaces, enabling it to be used across DLLs or independent subsystems. ABI-safe boundaries, allocator duplication patterns, and CRTP static dispatch ensure predictability and safety.

* **Zero Dynamic Overhead**
  All fast paths avoid virtual dispatch and dynamic allocation. Dispatch is static or manually bound. Lifecycle and execution logic is encoded via interfaces such as `IHandle`, `IExecutor`, `IExecutorVirtual`, `IWorkStealLoad`, and `IComposer`.

---

## Core Interfaces

* `IHandle` — Root handle abstraction for thread and task pool control
* `IExecutor` — Compile-time fast-path interface for execution
* `IExecutorVirtual` — Runtime-bound executor interface (plugin-style)
* `IWorkStealLoad` — Interface for stealable workloads
* `IComposer` — Declarative composition interface
* `ATaskEngine` — Deterministic batch-oriented execution core

---

## Design Goals

* **Determinism First**
  All task execution, scheduling, and lifecycle transitions are explicitly controlled — no implicit yields or dynamic scheduling decisions.

* **Cache and Allocator Locality**
  Memory and task execution are tightly bound to TLS-managed allocators, reducing contention and improving locality across cores.

* **Extensible and Replaceable**
  All components (e.g. pools, composers, workloads) can be used independently or replaced with custom implementations without modifying internals.

* **Debuggable Under Load**
  Blaze is built for systems where traceability matters: pool state, composition context, and scheduling behavior can all be instrumented with zero impact to fast paths.

---

## Setup & Build Instructions

### Requirements

* C++20-compliant compiler (GCC 12+, MSVC 2022+, Clang 14+)
* No external dependencies
* CMake 3.20+ (optional)

---

### Building with CMake

```bash
git clone https://github.com/your-org/blaze.git
cd blaze
mkdir build && cd build
cmake ..
cmake --build . --config Release
```

You can also embed Blaze directly into your own project by including the `include/` and `src/` folders and linking the Blaze core statically.

---

### Platform Support

* ✅ Windows 10/11 (x64)
* ✅ Linux (glibc, musl, x86\_64)
* ⛔ macOS (not officially supported or tested)
* 🔶 Cross-platform builds assume no reliance on OS-specific threading APIs outside POSIX/Win32

---
