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
#include "Corium.h"

namespace Corium::Utils{
    class CORIUM TaskOptions final {
	    
    };

    enum class CORIUM TaskState : uint8_t{
        PENDING, SCHEDULED, RUNNING, FAILED, CANCELLED, COMPLETE
    };

    enum class CORIUM WorkerState : uint8_t{
        INITIALIZING, SUSPENDED, RUNNING, TERMINATED
    };

    struct CORIUM IHandle{
		IHandle() = default;
		virtual ~IHandle() = default;

		IHandle(const IHandle&) = delete;
		IHandle& operator=(const IHandle&) = delete;

        [[nodiscard]] virtual TaskState getState() const noexcept = 0;
        [[nodiscard]] virtual size_t getTaskID() const noexcept= 0;
        [[nodiscard]] virtual bool cancelTask() noexcept = 0;

        template<typename T, typename Derived>
        T result() noexcept{
            static_assert(std::is_base_of_v<IHandle, Derived>, "Derived must be a subclass of IHandle");
            return static_cast<Derived*>(this)->template result<T>();
        }

        [[nodiscard]] virtual std::exception_ptr getException() const noexcept= 0;
    	virtual void rethrow() const = 0;
    };
}