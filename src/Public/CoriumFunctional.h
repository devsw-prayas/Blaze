/*
* Copyright (c) 2025 StormWeaver
*
* This file is part of the Blaze Multithreading API
*
* Licensed under the MIT License. You may obtain a copy of the License at
* https://opensource.org/licenses/MIT
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND...
*/

#pragma once
#include "Corium.h"

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
