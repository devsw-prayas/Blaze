# Corium Code Style Guide

## Motivation
Certain layout and formatting rules are recommended to keep the code readable, predictable, and to make peace with my Java-flavored instincts.

## A) Indentation and Bracing Style
1. Indentation is mandatory for all block-scoped code.
2. Range-based for loops with single-expression bodies may be written on a single line.
3. `if` statements with single-expression bodies may be inlined.
4. `else` statements with single-expression bodies may be inlined.
5. Lambdas may be inlined if their body is a single expression and fits clearly on one line.
6. All other constructs (classes, structs, functions, etc.) must always be braced and indented.
7. Braces always stay on the same line as the statement — this is K&R style, not Java vertical cliffs.

### Code Examples
```cpp
// Good: Single-line range-based for loop
for (auto& item : items) process(item);

// Good: Inlined if and else
if (is_valid) count++;
else log_error();

// Good: Single-expression lambda
auto square = [](int x) { return x * x; };

// Bad: Unbraced multi-statement if (don't do this!)
if (is_valid) count++; log("Updated"); // Chaos awaits.

// Good: Braced class with K&R style
class CoriumThing {
    void do_work() { process(); finalize(); }
};
```

## B) Line Length and Limits
1. A single file should not exceed 15,000 lines — including whitespace, breathers, and sanity breaks — unless absolutely necessary (e.g., auto-generated code, giant tables).
2. A line may extend to around 200 columns before wrapping is forced.

### Code Example
```cpp
// Good: Keep it under 200 columns
void process_data(const std::vector<int>& data, bool should_log, const std::string& prefix) { ... }

// Bad: This line is trying to win a marathon
void process_data_really_long_function_name_to_make_a_point_about_column_limits(const std::vector<int>& super_long_data_name, bool should_log_forever, const std::string& unnecessarily_verbose_prefix_name) { ... }
```

## C) Header / Source Separation
1. Unless required for inlining or templating, all implementation logic should live in a `.cpp` file.
2. Source files must always be kept inside a `Private` folder.
3. Headers must always be kept inside a `Public` folder.

### Code Example
```
// Public/CoriumThing.h
#pragma once
class CoriumThing {
public:
    void do_work();
};

// Private/CoriumThing.cpp
#include "CoriumThing.h"
void CoriumThing::do_work() {
    // Implementation lives here, not in the header!
}
```

## D) Function Length / Nesting Depth Limits
1. Functions should be limited to ~70 lines of code (excluding whitespace).
2. `if` blocks may be nested up to 16 levels — but if you reach that number, either you're writing a VM, or something’s very wrong.
3. `switch` statements may be nested if explicitly justified, up to 4 levels deep.
4. `goto` is banned. If you use it, you better be hand-writing `setjmp` macros — and even then, you should still feel guilty.

### Code Examples
```cpp
// Good: Short function
void process_item(int item) { // ~10 lines
    if (item > 0) {
        log("Positive");
        update(item);
    }
}

// Bad: Function longer than a CVS receipt
void process_everything() { // 200 lines of doom
    // ... imagine endless code here ...
}

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

## E) File-Per-Type Enforcement
1. This is not Java.
2. File names must reflect the abstract role of their contents. Group types where it makes sense — we’re not doing one-class-per-file unless it helps.
3. Utility types fundamental to larger types should be declared first in the file, before the dependent types.

### Code Example
```
// RendererUtils.h
struct RenderContext { /* Utility type first */ };
class Renderer { /* Depends on RenderContext */ };
```

## F) Member Order
1. Avoid scattering different specifier blocks (`public`, `private`, `protected`) throughout a class.
2. Recommended structure:
   - Private member variables at the top
   - Public-facing API in the middle
   - Protected and private methods toward the bottom
3. This rule doesn’t apply to structs — they may be organized for SoA layout, SIMD compatibility, or memory alignment. In those cases, performance wins over hierarchy.

### Code Example
```cpp
class CoriumProcessor {
private:
    int data_;
    double config_;

public:
    void process();
    int get_result() const;

protected:
    void helper();

private:
    void internal_work();
};

// Structs can ignore hierarchy for performance
struct AlignedData {
    alignas(16) float vec[4]; // SIMD-friendly
};
```

## G) Include Management
1. Forward declarations are discouraged — they rot, tangle, and create spaghetti faster than they solve build times.
2. All `.cpp` files must include the PCH header for their module.
3. Common macros, headers, and constants must be placed inside the module's PCH header, named after the module.

### Code Example
```
// CoriumCorePCH.h
#pragma once
#include <vector>
#include <string>
#define SPECTRA_LOG "LogPrefix"

// CoriumThing.cpp
#include "CoriumCorePCH.h"
// No forward declarations, no mess
```