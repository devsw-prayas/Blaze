#pragma once
#include <Corium.h>

namespace Corium::Core::Utils {
    template<typename A, typename S>
    struct ClosureFunction;

    template<typename A, typename R, typename... Args>
    struct ClosureFunction<A, R(Args...)> {
        using AllocatorType = A;
        using EntryType = R(*)(void*, Args...);

        void* m_Context = nullptr;
        EntryType m_Entry = nullptr;

        ClosureFunction(const ClosureFunction&) = default;
        ClosureFunction& operator=(const ClosureFunction&) = default;
        ClosureFunction(ClosureFunction&&) noexcept = default;
        ClosureFunction& operator=(ClosureFunction&&) noexcept = default;

        ~ClosureFunction() = default;

        // --- trampoline (must be static)
        template<typename L>
        static R lambdaInvoke(void* ctx, Args... args) {
            return (*static_cast<L*>(ctx))(std::forward<Args>(args)...);
        }

        // --- constructor from lambda
        template<typename L> requires !std::is_same_v<std::remove_cvref_t<L>, ClosureFunction>
        explicit ClosureFunction(L&& lambda) {
            using LambdaT = std::decay_t<L>;

            LambdaT* stored = AllocatorType::template allocate<LambdaT>(
                std::forward<L>(lambda)
            );

            m_Context = stored;
            m_Entry = &lambdaInvoke<LambdaT>;
        }

        R operator()(Args... args) const {
            return m_Entry(m_Context, std::forward<Args>(args)...);
        }
    };


}
