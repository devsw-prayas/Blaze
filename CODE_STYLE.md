# Corium Style Manifesto
> Im lazy, so all subsytems get the copy

## Why We Write Code This Way
Style guides are usually about "cleanliness," but in a renderer, style is about **navigation and density**. We work with large files, complex templates, and low-level data. The following rules are designed to keep the logic visible and the data accessible.

## 1. Class Structure: Data First
We put private member variables at the **top** of the class.
*   **The "Why":** I don't want to scroll through 1,000 lines of implementation logic just to find the private state of a class. The data is the most important part of the object—it should be the first thing you see.

### Member Order Example
```cpp
class SpectraProcessor {
private:
    int m_data;         // Data layout first!
    double m_config;

public:
    void process();     // Public API in the middle
    int getResult() const;

protected:
    void helper();      // Internals at the bottom

private:
    void internalWork();
};

// Structs can ignore hierarchy for performance (SIMD/Alignment)
struct AlignedData {
    alignas(16) float vec[4]; 
};
```

## 2. Bracing: The Java Way
We use K&R style braces (opening brace on the same line as the statement).
*   **The "Why":** This is pure Java nostalgia. It keeps the code dense and prevents what I call "vertical cliffs"—where you spend half your time scrolling past empty lines with single braces.

### Bracing & Inlining Examples
```cpp
// Good: K&R style
void doWork() {
    if (isValid) {
        process();
    }
}

// Good: Single-line inlining for simple logic
for (auto& item : items) process(item);
if (isValid) count++;
else logError();

// Good: Single-expression lambda
auto l_square = [](int x) { return x * x; };
```

## 3. Line Length: Ultra-Wide
Lines are allowed to extend up to **200 columns** before wrapping.
*   **The "Why":** Between verbose namespace names, deeply nested templates, and long graphics API calls (Vulkan/D3D12), 80 or 120 characters just isn't enough. We have wide monitors for a reason.

### Column Limit Example
```cpp
// Good: Keep it under 200 columns to avoid horizontal 'staircases'
void processData(const std::vector<int>& r_Data, bool v_ShouldLog, const std::string& ro_Prefix) { ... }

// Bad: This line is trying to win a marathon (still keep names sane)
void processDataReallyLongFunctionNameThatDoesTooMuch(const std::vector<int>& superLongDataName, bool shouldLogForever) { ... }
```

## 4. File Limits: Mega-Files are Fine
A single file can grow up to **15,000 lines**.
*   **The "Why":** We often need large Look-Up Tables (LUTs) or massive blocks of documentation and comments embedded directly in the source. I'd rather have everything related to one module in a single searchable file.

## 5. Forward Declarations: The Local Exception
Global forward declarations are generally discouraged.
*   **The "Why":** They tend to "rot and tangle," leading to cyclic builds and a nightmare of "incomplete type" errors. 
*   **The Exception:** Use them locally inside a namespace (like in the VulkanBackend bootstrap) when required by specific patterns.

## 6. Logic & Control Flow
*   **Nesting:** You can nest `if` blocks up to 16 levels. If you reach that number, you're either writing a VM or something is very wrong.
*   **No Goto:** `goto` is banned. It's spaghetti code.
*   **Functions:** Keep them around ~70 lines of code. If it's longer than a CVS receipt, break it up.

### Nesting Hell Example
```cpp
// Bad: Nesting hell (don’t do this)
void nightmare() {
    if (cond1) {
        if (cond2) {
            if (cond3) { // 16 levels deep? Send help.
                goto escape; // BANNED!
            }
        }
    }
}
```

## 7. Header/Source Separation
Keep your headers clean. 
*   **Public/** for headers, **Private/** for implementations. 
*   Unless it’s a template or an inline performance hint, the logic stays in the `.cpp`.

### File Structure Example
```
// Public/SpectraThing.h
#pragma once
class SpectraThing {
public:
    void doWork();
};

// Private/SpectraThing.cpp
#include "SpectraThing.h"
void SpectraThing::doWork() {
    // Implementation lives here!
}
```

## The Golden Rule
If a style rule makes it harder to read a complex shader-binding table or a critical hot-loop, **break it**. The goal is a high-performance renderer, not a pretty-print contest.
