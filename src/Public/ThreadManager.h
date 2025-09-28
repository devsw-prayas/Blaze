#pragma once
#include "ThreadPlatform.h"

namespace Corium::Manager {
	class IThreadManager {
	public:
		IThreadManager() = default;
		IThreadManager(const IThreadManager&) = delete;
		IThreadManager& operator=(const IThreadManager&) = delete;
		IThreadManager(IThreadManager&&) noexcept = default;
		IThreadManager& operator=(IThreadManager&&) noexcept = default;

		template<typename F, typename D>
		Platform::ThreadHandle launch(F&& u_Function) {
			return static_cast<D*>(this)->template launchC<F>(std::forward<F>(u_Function));
		}

		virtual bool isRunning(Platform::ThreadHandle v_Handle) = 0;
		virtual bool kill(Platform::ThreadHandle v_Handle) = 0;
		virtual bool join(Platform::ThreadHandle v_handle) = 0;
	protected:
		virtual ~IThreadManager() = default;
	};
}
