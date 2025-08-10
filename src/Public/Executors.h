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
#include "Blaze.h"
#include "BlazeMemory.h"
#include "BlazeUtils.h"

namespace Blaze::Executors {
    template<typename Derived>
    class BLAZE IExecutor{
    public:
        void execute(void* (*pf_Func)()){
            static_cast<Derived*>(this)->execute(pf_Func);
        }

        template<typename F, typename...Args>
        void execute(F&& u_Func, Args&&...u_Args){
            static_cast<Derived*>(this)->execute(std::forward<F>(u_Func), std::forward<Args>(u_Args)...);
        }
    };

    class BLAZE IExecutorVirtual {
    public:
		IExecutorVirtual() = default;
		virtual ~IExecutorVirtual() = default;

		IExecutorVirtual(const IExecutorVirtual&) = delete;
		IExecutorVirtual& operator=(const IExecutorVirtual&) = delete;

		IExecutorVirtual(const IExecutorVirtual&&) = delete;
		IExecutorVirtual& operator=(const IExecutorVirtual&&) = delete;

        virtual void execute(std::function<void()> l_func) = 0;

		[[nodiscard]] virtual Memory::SharedPointer<Utils::IHandle> submitV(std::function<void()> l_Func,
			const Utils::TaskOptions& r_Options) = 0;

        [[nodiscard]] virtual Memory::SharedPointer<Utils::IHandle> callV(std::function<void()> l_Func,
            const Utils::TaskOptions& r_Options) = 0;

        [[nodiscard]] virtual std::vector<Memory::SharedPointer<Utils::IHandle>>
            submitBatchV(std::vector<std::function<void()>> l_Func, std::vector<const Utils::TaskOptions&> r_Options) = 0;

        [[nodiscard]] virtual std::vector<Memory::SharedPointer<Utils::IHandle>>
            callBatchV(std::vector<std::function<void()>> l_Func, std::vector<const Utils::TaskOptions&> r_Options) = 0;

        [[nodiscard]] virtual bool isRunning() const noexcept = 0;

		[[nodiscard]] virtual bool isShutdown() const noexcept = 0;

        virtual bool awaitTermination(std::chrono::steady_clock::duration v_Duration) = 0;

		virtual void shutdown() noexcept = 0;
		virtual void shutdownNow() noexcept = 0;
    	virtual bool cancelAllPending() noexcept = 0;

		[[nodiscard]] virtual bool isQuiescent() const noexcept = 0;
        [[nodiscard]] virtual size_t getActiveTaskCount() const noexcept = 0;
        [[nodiscard]] virtual size_t getCompletedTasksCount() const noexcept = 0;
        [[nodiscard]] virtual size_t getCancelledTasksCount() const noexcept = 0;
    };
}
