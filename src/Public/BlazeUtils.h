#pragma once
#include "Blaze.h"

namespace Blaze::Utils{
    class BLAZE TaskOptions final {
	    
    };

    enum class BLAZE TaskState : uint8_t{
        PENDING, SCHEDULED, RUNNING, FAILED, CANCELLED, COMPLETE
    };

    enum class BLAZE WorkerState : uint8_t{
        INITIALIZING, SUSPENDED, RUNNING, TERMINATED
    };

    struct BLAZE IHandle{
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