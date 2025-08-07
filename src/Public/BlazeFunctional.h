#pragma once
#include "Blaze.h"

namespace Blaze::Functional {
    template<typename F>
    class Functional final {
        F m_f;
    public:
        constexpr Functional(F&& u_Func) : m_f(std::forward<F>(u_Func)) {}

        template<typename ...Args> requires std::invocable<F&, Args...>
        decltype(auto) operator()(Args&&...u_Args) {
            return m_f(std::forward<Args>(u_Args)...);
        }

        template<typename ...Args>
        decltype(auto) operator()(Args&&...u_Args) const {
            return m_f(std::forward<Args>(u_Args)...);
        }
    };

    template<typename F>
    Functional(F) -> Functional < std::decay_t<F>>;

    template<typename F>
    auto makeFunctional(F&& u_Func) {
        return Functional<F>{std::forward<F>(u_Func)};
    }
}
