# Corium Naming Manifesto
> Im lazy, so all subsytems get the copy

## Why We Name Things This Way
Coding standards usually feel like they were written by a robot for a robot. This one isn't. The naming conventions in Spectra are born out of driver-level debugging, the quirks of C-style ABIs, and a bit of Java nostalgia. 

The goal is **Visual Grep**: you should be able to see the memory layout, the ownership, and the intent of a variable just by looking at its name, without needing an IDE to save you.

## 1. The "Visual ABI" (Parameters & High-Level Variables)
In a renderer, a `float*` is never just a `float*`. The compiler sees an address; we see a memory contract.

### Pointer & Reference Prefixes
We use these prefixes to explicitly communicate behavior and prevent logic errors at the "decay" level.

| Prefix | Meaning | The "Why" |
| :--- | :--- | :--- |
| `p_` | **Pointer to Value** | Direct address-based passing for single values. |
| `pa_` | **Pointer to Array** | Used when passing arrays to C-style functions. It warns that decay has happened. |
| `po_` | **Pointer to Object** | Distinguishes a heap-allocated object from a raw data address. |
| `r_` | **Reference to Value** | For trivial, handle-based aliases (struct-wrapped `uint64_t`). |
| `ra_` | **Reference to Array** | Used for fixed-size array references. |
| `ro_` | **Reference to Object** | For heavy descriptors or stateful objects. |

### Smart Pointers: The Ownership Guard
*   `su_` (unique), `ss_` (shared), `sw_` (weak).
*   **The "Why":** Since we often cast to raw pointers for performance, we need these prefixes to stop us from accidentally messing with ownership. 

### The "Meyers Rule" (Universal References)
*   `u_` (Universal Reference).
*   **The "Why":** Inspired by Scott Meyers (Effective Modern C++). A `u_` prefix reminds you that this is a forwarding reference that **must** be `std::forward`ed.

### Example: Full Parameter Prefixing
```cpp
void processData(
    int v_Value,              // Pass-by-value
    float* p_Pointer,         // Pointer to value
    float* pa_Array,          // Pointer to array (decayed)
    SpectraObject* po_Object, // Pointer to object
    int& r_Handle,            // Reference to value (handle alias)
    Descriptor& ro_Desc,      // Reference to heavy object
    std::unique_ptr<T> su_Ptr // Unique ownership
);
```

**Note on Locals:** We don't use `p_` or `r_` for local variables. Locals should be clean `camelCase`. 

## 2. Type Naming & Hierarchy (The Nostalgia Trip)
*   **PascalCase** for all classes and structs.
*   **Interfaces (`I`):** Pure virtual bases.
*   **Abstracts (`A`):** Partially implemented bases.
    *   **The "Why":** Java nostalgia. It makes scanning a directory list instant.

### Type Examples
```cpp
class IRenderer { virtual void render() = 0; };
class ARendererBase : public IRenderer { /* Partial impl */ };
class VulkanRenderer : public ARendererBase { /* Concrete impl */ };

enum class Color : uint8_t { Red, Green, Blue };
```

## 3. Scoping & Storage
*   `m_memberVariable`: Private members. (e.g., `int m_count;`).
*   `g_GlobalVariable`: Globals (PascalCase + prefix). (e.g., `int g_GlobalCount;`).
*   `t_ThreadLocal`: Thread-local data. (e.g., `thread_local int t_ThreadId;`).
*   `s_staticVariable`: Class-level statics. (e.g., `static int s_staticCounter;`).

## 4. Templates: The Clarity Alias
Template parameters (`T`, `U`) are great for the compiler but terrible for the human.

1.  Use `T`, `U`, `V` in the declaration.
2.  **Immediately** alias them inside the class using `_camelCase` with a leading underscore.

### Template Example
```cpp
template<typename T>
class Buffer {
private:
    using _dataType = T; // Much easier to read than 'T' everywhere
};

template<int Size>
class FixedArray {}; // Non-type params are PascalCase
```

## 5. Macros: The Module Guard
*   **Format:** `SCREAMING_SNAKE_CASE` with a `MODULE_` prefix.
*   **The "Why":** Prevents chaos and name collisions between different backends.
```cpp
#define CORE_LOG_LEVEL 3
#define VULKAN_ASSERT(cond) if (!(cond)) { ... }
```

## 6. The "Boundary Rule"
*   **Engine Internals:** Full prefixing (`ro_`, `pa_`, `su_`) is mandatory. 
*   **Public APIs / Utilities:** Prefixes can be relaxed to simple `v_`, `p_`, `r_` if the intent is obvious. 

## The Golden Rule
If a naming rule makes the code harder to read in a specific, performance-critical edge case—**break it**. But if you're just being lazy, stick to the manifesto.
