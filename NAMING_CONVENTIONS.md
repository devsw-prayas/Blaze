# Coding Conventions
## Motivation:
All code written across the entire development cycle must adhere strictly to the following rules and conventions to ensure consistency, maintainability, and correctness across the entire codebase. These conventions act as a structural contract for all contributors and enforce clear communication between systems and abstraction layers.

## [Naming Conventions]
### A) Type Naming
1. All `class`  and `struct`  that define concrete implementations must use `PascalCase`.
   ```cpp
   class ConcreteClass { /* ... */ };
   struct DataStruct { /* ... */ };
   ```
2. Purely virtual `class` implementations (interfaces) must be prefixed with a capital I.
   ```cpp
   class IInterface {
   public:
       virtual void doSomething() = 0;
   };
   ```
3. Abstract `class` (partially virtual) must be prefixed with a capital A.
   ```cpp
   class AAbstractBase {
   public:
       virtual void partialImplementation() = 0;
       void concreteMethod() { /* ... */ }
   };
   ```
4. `enum` must always be built using `enum class` and must use `uint8_t` as the underlying type unless a wider type is explicitly justified.
   ```cpp
   enum class Color : uint8_t { Red, Green, Blue };
   ```
5. Usage of `union ` is prohibited. Tagged struct s or standard variants must be used instead.
   ```cpp
   struct TaggedStruct {
       bool isInt;
       int intValue;
       double doubleValue;
   };
   // Instead of: union { int a; double b; };
   ```

### B) Function and Method Names
1. All function and method names must follow `camelCase`.
   ```cpp
   void processData();
   class Example {
       void calculateResult();
   };
   ```
2. Functions that reflect internal or thread-local behavior may be prefixed with internal or current, as appropriate.
   ```cpp
   void internalProcessData();
   int currentThreadId();
   ```

### C) Variable Naming
1. Local variables must follow `camelCase`, and may optionally use a trailing underscore `_` to prevent name shadowing.
   ```cpp
   int localVariable;
   int localVariable_; // To avoid shadowing
   ```
2. Private member variables must be prefixed with m_ and follow `camelCase`.
   ```cpp
   class Example {
   private:
       int m_memberVariable;
   };
   ```
3. Global variables must be prefixed with `g_` and use `PascalCase`.
   ```cpp
   int g_GlobalCounter;
   ```
4. `thread_local` variables must be prefixed with `t_` and use `PascalCase`.
   ```cpp
   thread_local int t_ThreadId;
   ```
5. `static` variables within a class must be prefixed with `s_` and use `camelCase`.
   ```cpp
   class Example {
   private:
       static int s_staticCounter;
   };
   ```
6. `static` variables inside a function can use regular `camelCase` without prefixing.
   ```cpp
   void exampleFunction() {
       static int staticCounter;
   }
   ```
7. The naming evaluation order is: scope → storage duration/type → naming rule.

### D) Constants and Macros
1. Constants must use `constexpr` and be written in `SCREAMING_SNAKE_CASE`.
   ```cpp
   constexpr int MAX_COUNT = 100;
   ```
2. Macros must be capitalized and use underscores. Optionally, macros can be prefixed with a module code for clarity.
   ```cpp
   #define MODULE_LOG_LEVEL 3
   ```
3. Constants ignore prefix rules and must be self-descriptive in name and meaning.

### E) Template Parameters
1. Template type parameters must be single uppercase letters (`T`, `U`, `K`, etc.). For variadic templates, use `Args`.
   ```cpp
   template<typename T, typename U>
   class Pair;
   template<typename... Args>
   class Variadic;
   ```
2. All template parameters must be converted into `using` declarations scoped privately inside the class, written in `PascalCase` to denote placeholders.
   ```cpp
   template<typename T>
   class Example {
   private:
       using PascalCase = T;
   };
   ```
3. Template non-type parameters (e.g., int N) must be Capitalized.
   ```cpp
   template<int Size>
   class FixedArray {};
   ```

### F) Namespaces
1. All `namespace` must be capitalized using PascalCase.
   ```cpp
   namespace CoreSystem {
       // ...
   }
   ```
2. `namespace` that include `thread_local` data must define a `this_thread` sub-namespace to group them.
   ```cpp
   namespace CoreSystem {
       namespace this_thread {
           thread_local int t_ThreadData;
       }
   }
   ```

### G) Lambdas
1. Lambda expressions must be assigned to variables prefixed with `l_`.
   ```cpp
   auto l_process = []() { /* ... */ };
   ```
2. Lambda variables are exempt from further prefixing or ownership markers.

### H) References and Pointers
1. All references must be prefixed with `r_`, followed by a `PascalCase` name.
   ```cpp
   int& r_DataReference = someVariable;
   ```
2. All raw pointers must be prefixed with `p_`, followed by a `PascalCase` name.
   ```cpp
   int* p_DataPointer = nullptr;
   ```
3. All universal references must be prefixed with `u_`, followed by a `PascalCase` name.
   ```cpp
   template<typename T>
   void process(T&& u_UniversalData);
   ```

### I) Function Parameters (Prefixing)
1. Functions do not require suffixing, but function parameters must be strictly prefixed to indicate their type of binding and ownership.

   Type of Binding | Prefix
   --- | ---
   Pass-by-value | `v_`
   Pointer | `p_`
   Pointer to array | `pa_`
   Pointer to function | `pf_`
   Pointer to object | `po_`
   Universal reference | `u_`
   Reference | `r_`
   Reference to array | `ra_`
   Reference to function | `rf_`
   Reference to object | `ro_`
   std::unique_ptr | `su_`
   std::shared_ptr | `ss_`
   std::weak_ptr | `sw_`

   ```cpp
   void processData(int v_Value, int* p_Pointer, int& r_Reference,
                    std::unique_ptr<int> su_Unique, std::shared_ptr<int> ss_Shared);
   ```

### Parameter Prefixing Policy
Parameter prefixing is a structural design contract.
It explicitly communicates the ownership, lifetime, and calling semantics of arguments. Its application depends on the abstraction level, runtime criticality, and exposure of the API.

#### When Full Prefixing is Required
Full parameter prefixing (`ro`_, `pa_`, `pf_`, etc.) is mandatory when:

1. Writing code that interacts with low-level systems such as:
   - Memory management
   - Threading, task, or job scheduling
   - Synchronization primitives
2. Ownership, reference semantics, or allocation behavior must be unambiguous
3. The API is not exposed to higher-level systems or users and requires strict control over argument behavior

Such enforcement ensures:
- No implicit copies or aliasing violations
- No hidden heap allocations
- Memory lifetime and ownership remain explicitly traceable

#### When Prefixing May Berelaxed
Simplified prefixing (e.g., `v_`, `r_`, `p_`, `u_`) is allowed when:
1. Writing ergonomic or user-facing interfaces
2. Creating helper utilities or debug code
3. Working in layers where function semantics are self-evident
4. Argument types are clear from context or naming

#### Boundary Rule
If a function bridges two layers (e.g., receives high-level input and dispatches it to low-level systems):
1. The exposed interface may use minimal prefixing
2. Internally, the low-level component must enforce strict prefixing rules

#### Summary
Layer / Context | Prefixing Policy
--- | ---
Low-level, memory, threading | Full (`ro_`, `pa_`, `pf_`, …)
High-level, utility, public | Minimal (`v_`, `p_`, `r_`, `u_`)
Boundary APIs | Minimal in interface, strict internally

Note: Prefixing is not a stylistic concern — it is a semantic and architectural guarantee that helps enforce data discipline across engine layers.

### J) File and Directory Structure
1. Each component or module must follow this structure:
   ```plaintext
   /ModuleName/
       └── src/
           ├── Public/     → For all headers (.h, .inl, .impl)
           └── Private/    → For implementation files (.cpp)
   ```
2. No global path hierarchy is imposed
3. Only the Public/ folder may expose interfaces to other modules
4. All internal code and logic must reside in Private/
5. Implementation logic may be split into `.h`, `.cpp`, and `.impl`/`.inl` as needed for inlining or templating
   ```plaintext
   /CoreSystem/
       └── src/
           ├── Public/
           │   ├── IInterface.h
           │   ├── CoreTypes.h
           └── Private/
               ├── CoreTypes.cpp
               ├── Implementation.impl
   ```