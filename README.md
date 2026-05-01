# Corium

A deterministic, high-performance execution runtime for C++. Provides explicit control over task orchestration, thread scheduling, GPU dispatch, and memory lifetimes — built from scratch with zero-overhead execution paths and no external runtime dependencies.

Corium is the concurrency and execution substrate for [SpectraRenderer](https://github.com/devsw-prayas/Spectra), and is designed to be used independently as well.


## What it does

Corium replaces conventional concurrency abstractions (`std::async`, thread pools, task libraries) with an execution model built around three hard invariants:

- **Explicit.** Tasks are inert descriptors until submitted. No implicit scheduling, no hidden allocation.
- **Deterministic.** Execution order, memory layout, and task lifecycle transitions are all explicitly controlled.
- **Zero overhead on the fast path.** All performance-critical execution resolves statically — no virtual dispatch, no dynamic allocation in hot paths.


## Task Pipeline

Every task passes through three ordered phases. Phase boundaries are enforced at compile time and runtime.

```
Phase 1 — Induction      Declare schema: parameter types, memory footprint, trait flags
Phase 2 — Construction   Bind executable closure (CPU lambda or GPU kernel) to the task
Phase 3 — Execution      Stage parameter values, assign TaskID, submit to pool
```

**Phase 1 — `TaskInductor`**  
Operates on a stack-allocated `TaskDesc`. Declares input/output parameter schemas via `TensorParameter<T, Extents...>`, encodes execution traits via `TaskBitFlag`, and calls `cook()` to freeze the descriptor and produce a `TaskFrame`.

**Phase 2 — `TaskBuilder`**  
Operates on frozen `TaskDesc` objects. Binds callables (`void(TaskContext&)`), sliced callables (`void(TaskSliceContext&)`), or GPU kernels (`GPUKernel`) to the `TaskFrame`.

**Phase 3 — `Executor`**  
Stages typed input values into the task's packed buffer, assigns a `TaskID`, and routes the frame to the appropriate pool. Returns a `TaskHandle` for observable submissions.

## Pool Hierarchy

| Pool | Description |
|---|---|
| `ThreadExecutorService` | Managed CPU worker thread pool. Baseline execution path. |
| `WorkStealerService` | Extends `ThreadExecutorService` with per-worker local deques and work stealing. |
| `ScheduledThreadExecutorService` | Adds time-based deferred and periodic submission. |
| `AcceleratorService` | CPU + GPU. Owns a dedicated GPU monitor thread and CUDA execution resources. |

Pool bindings are managed via `ExecutionContract<PoolT>` — task-pool compatibility is validated entirely at compile time against `TaskBitFlag` traits.


## GPU Execution

`AcceleratorService` owns one GPU monitor thread per pool instance. CPU workers hand off GPU `TaskFrame`s via a lockless queue. The monitor thread handles device transfer, kernel dispatch, stream polling, and completion.

GPU kernels are compiled and bound via `GPUKernel`:

```cpp
// Compile from source at runtime (NVRTC)
GPUKernel kernel = GPUKernel::fromSource(ptxSource, "myKernel");

// Load pre-compiled PTX
GPUKernel kernel = GPUKernel::fromPtx(ptxBlob, "myKernel");

// Load from file
GPUKernel kernel = GPUKernel::fromFile("path/to/kernel.ptx", "myKernel");
```

Kernels are bound to tasks via `TaskBuilder::bindGPU()`. All grid/block dimensions and shared memory requirements are validated at bind time against live device properties — no silent fallback.

CPU fallback is automatic unless `ForbidsSoftwareFallback` is set on the task.


## Synchronization Primitives

All primitives in `Corium::Sync`. Hybrid wait strategy — userspace spin for short waits, OS primitive fallback for long waits.

| Primitive | Description |
|---|---|
| `CountDownLatch` | One-shot. N countdowns unblock all waiters. |
| `CyclicBarrier` | Reusable. N parties must arrive before any proceed. Optional trip hook. |
| `Semaphore` | Permit-based resource control. |
| `ReentrantLock` | Explicit mutex with reentrant semantics. Pairs with `Condition`. |
| `ReadWriteLock` | Concurrent readers or exclusive writers. Upgradeable. |
| `StampedLock` | Optimistic read variant — validate-before-lock pattern. |
| `Phaser` | Dynamic barrier. Parties register/deregister at runtime. Hierarchical. |
| `Exchanger<T>` | Two-thread rendezvous and value swap. |

Free functions for `TaskHandle` sets: `Sync::waitAll`, `Sync::waitAny`.


## Memory Layout

Corium reserves a 512 GiB virtual address space across up to 4 NUMA nodes (128 GiB per node). Physical page affinity is set at reserve time via `VirtualAllocExNuma`. Each node is an independent VA reservation — no shared base.

Per-node regions:

| Region | Size | Contents |
|---|---|---|
| `g_RuntimeVA` | 24 GiB | Closures, smart pointer control blocks, pools, allocators |
| `g_TaskMetadataVA` | 32 GiB | TaskMemoryDesc, TaskContext, parameter layout arrays |
| `g_TaskPayloadVA` | 56 GiB | Input/output payload buffers, GPU-visible task data |
| `g_ReservedVA` | ~16 GiB | GPU staging, GPUKernel storage, DMA windows |

Guard pages between all regions. Worker threads are NUMA-bound — allocations stay node-local.

## Composer API

High-level convenience layer that hides the three-phase pipeline. Suitable for non-critical paths.

```cpp
// Fire and forget
Composer::run([](TaskContext& ctx) { /* work */ });

// Observable async
auto handle = Composer::async<MyIO>(inputA, inputB,
    [](TaskContext& ctx) { ctx.put<Result>(compute(ctx.get<A>(), ctx.get<B>())); });

// GPU dispatch with CPU fallback
auto handle = Composer::kernel<MyIO>(inputA, inputB, gpuKernel,
    [](TaskContext& ctx) { /* cpu fallback */ });
```


## Build

### Requirements

- Windows 10/11 (x64) or Linux (x86_64)
- Visual Studio 2022 (MSVC) on Windows, GCC 12+ or Clang 14+ on Linux
- CMake 3.20+
- CUDA Toolkit 12.4+ (required for GPU execution path)

### Standalone

**Windows:**
```bash
git clone https://github.com/devsw-prayas/Corium.git
cd Corium
mkdir build && cd build
cmake .. -G "Visual Studio 17 2022"
cmake --build . --config Release_win64
```

**Linux:**
```bash
git clone https://github.com/devsw-prayas/Corium.git
cd Corium
mkdir build && cd build
cmake .. -G "Ninja"
cmake --build . --config Release
```

### As part of SpectraRenderer

Corium is a git submodule in `common/Corium`. Follow the [SpectraRenderer build instructions](../../README.md


## License

MIT License. See [LICENSE](LICENSE) for details.
