/*
* Copyright (c) 2025 StormWeaver
*
* This file is part of the Corium Multithreading API
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
#include <Corium.h>

namespace Corium::Core::Utils {
	template<typename R, typename... Args>
	struct Trampoline {
		template<typename L>
		static R invoke(void* ctx, Args... args) {
			return (*static_cast<L*>(ctx))(std::forward<Args>(args)...);
		}
	};

	template<typename A, typename S>
	struct ClosureFunction;

	template<typename A, typename R, typename... Args>
	struct ClosureFunction<A, R(Args...)> {
		using AllocatorType = A;
		using EntryType = R(*)(void*, Args...);

		void* m_Context = nullptr;
		EntryType m_Entry = nullptr;

		ClosureFunction() = default;
		ClosureFunction(const ClosureFunction&) = delete;
		ClosureFunction& operator=(const ClosureFunction&) = delete;

		ClosureFunction(ClosureFunction&& u_Other) noexcept
			: m_Context(u_Other.m_Context), m_Entry(u_Other.m_Entry) {
			u_Other.m_Context = nullptr;
			u_Other.m_Entry = nullptr;
		}

		ClosureFunction& operator=(ClosureFunction&& u_Other) noexcept {
			m_Context = u_Other.m_Context;
			m_Entry = u_Other.m_Entry;
			u_Other.m_Context = nullptr;
			u_Other.m_Entry = nullptr;
			return *this;
		}

		~ClosureFunction() = default;

		template<typename L>
			requires (!std::is_same_v<std::remove_cvref_t<L>, ClosureFunction>)
		explicit ClosureFunction(L&& lambda, AllocatorType* allocator) {
			if (!allocator) return;
			using LambdaT = std::decay_t<L>;

			LambdaT* stored = allocator->template emplace<LambdaT>(
				std::forward<L>(lambda)
			);

			m_Context = stored;
			m_Entry = &Trampoline<R, Args...>::template invoke<LambdaT>;
		}

		CORIUM_NODISCARD R operator()(Args... args) const {
			return m_Entry(m_Context, std::forward<Args>(args)...);
		}

		bool isCallable() const {
			return m_Context != nullptr && m_Entry != nullptr;
		}
	};

	template<typename>
	struct FunctionView;

	template<typename R, typename... Args>
	struct FunctionView<R(Args...)> {
		using Entry = R(*)(void*, Args...);

		void* m_Context = nullptr;
		Entry m_Entry = nullptr;

		template<typename L>
		FunctionView(L& lambda) {
			using LambdaT = std::remove_reference_t<L>;

			m_Context = &lambda;
			m_Entry = &Trampoline<R, Args...>::template invoke<LambdaT>;
		}

		// also allow view from ClosureFunction
		template<typename A>
		explicit FunctionView(const ClosureFunction<A, R(Args...)>& fn) {
			m_Context = fn.m_Context;
			m_Entry = fn.m_Entry;
		}

		CORIUM_NODISCARD R operator()(Args... args) const {
			return m_Entry(m_Context, std::forward<Args>(args)...);
		}
	};
}
