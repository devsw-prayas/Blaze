#pragma once
#include "Corium.h"

namespace Corium::Environment {
	struct CORIUM_RUNTIME_API CORIUM_ALIGNAS(32) CpuInfo final {
		uint32_t m_LogicalCoreCount = 0;
		uint32_t m_PhysicalCoreCount = 0;
		uint32_t m_NumaNodeCount = 0;
		uint32_t m_L1CacheSize = 0;
		uint32_t m_L2CacheSize = 0;
		uint32_t m_L3CacheSize = 0;
		uint32_t m_CacheLineSize = 0;

		CpuInfo() = default;
		~CpuInfo() = default;

		CpuInfo(const CpuInfo&) = default;
		CpuInfo& operator=(const CpuInfo&) = default;

		CpuInfo(CpuInfo&&) noexcept = default;
		CpuInfo& operator=(CpuInfo&&) noexcept = default;
	};

	struct CORIUM_RUNTIME_API CORIUM_ALIGNAS(8) VectorizeCapabilities final {
		uint8_t m_VectorCapabilities = 0;
		// bit-0  SSE
		// bit-1  SSE 4.1
		// bit-2  AVX
		// bit-3  AVX2
		// bit-4  AVX-512f

		VectorizeCapabilities() = default;
		~VectorizeCapabilities() = default;

		VectorizeCapabilities(const VectorizeCapabilities&) = default;
		VectorizeCapabilities& operator=(const VectorizeCapabilities&) = default;

		VectorizeCapabilities(VectorizeCapabilities&&) noexcept = default;
		VectorizeCapabilities& operator=(VectorizeCapabilities&&) noexcept = default;
	};

	struct CORIUM_RUNTIME_API CORIUM_ALIGNAS(32) OsInfo final {
		const char* m_ProductName = nullptr;
		uint32_t    m_MajorVersion = 0;
		uint32_t    m_MinorVersion = 0;
		uint32_t    m_BuildNumber = 0;

		OsInfo() = default;
		~OsInfo() = default;

		OsInfo(const OsInfo&) = default;
		OsInfo& operator=(const OsInfo&) = default;

		OsInfo(OsInfo&&) noexcept = default;
		OsInfo& operator=(OsInfo&&) noexcept = default;
	};

	// EnvironmentProbe — queries CPU topology, SIMD caps, and OS version once.
	// Must call init() before any getter.

	class CORIUM_RUNTIME_API EnvironmentProbe final {
	public:
		static void init();

		static const CpuInfo& getCpuInfo();
		static const VectorizeCapabilities& getVectorizeCapabilities();
		static const OsInfo& getOsInfo();
	};

	// CoriumProcess — lightweight process-level utilities.

	class CORIUM_RUNTIME_API CoriumProcess final {
	public:
		static uint32_t    getCurrentProcessId();
		static const char* getExecutablePath();
		static const char* getWorkingDirectory();
		static size_t      getEnvironmentVariable(const char* p_Name, char* p_Buffer, unsigned long v_BufferSize);
		static bool        setEnvironmentVariable(const char* p_Name, const char* p_Value);
	};

	// CoriumTermination — hard termination with an optional reason string.

	class CORIUM_RUNTIME_API CoriumTermination final {
	public:
		CORIUM_NORETURN static void terminate(const char* p_Reason, const char* p_FileName, int v_Line);
	};
}
