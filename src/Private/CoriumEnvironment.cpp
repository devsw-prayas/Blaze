#include "Corium.h"
#include "CoriumEnvironment.h"
#define ALLOW_SYSCALL
#include "CoriumSyscalls.h"

#if defined(_MSC_VER)
#include <intrin.h>
#elif defined(__GNUC__) || defined(__clang__)
#include <cpuid.h>
#endif

namespace Corium::Environment {
	namespace {
		bool                 g_IsInitialized = false;
		CpuInfo              g_CpuInfo;
		VectorizeCapabilities g_VectorizeCapabilities;
		OsInfo               g_OsInfo;
	}

	void EnvironmentProbe::init() {
		if (g_IsInitialized) return;

		g_CpuInfo = {};
		g_VectorizeCapabilities = {};
		g_OsInfo = {};

#ifdef _WIN32

		// CPU TOPOLOGY + CACHE

		DWORD size = 0;
		GetLogicalProcessorInformationEx(RelationAll, nullptr, &size);

		BYTE* buffer = static_cast<BYTE*>(malloc(size));
		if (!buffer) return;

		if (!GetLogicalProcessorInformationEx(
			RelationAll,
			reinterpret_cast<SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX*>(buffer),
			&size)) {
			free(buffer);
			return;
		}

		BYTE* p = buffer;
		BYTE* end = buffer + size;

		while (p < end) {
			auto* entry = reinterpret_cast<SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX*>(p);

			switch (entry->Relationship) {
			case RelationProcessorCore:
				{
					g_CpuInfo.m_PhysicalCoreCount++;

					const auto& proc = entry->Processor;
					for (WORD i = 0; i < proc.GroupCount; ++i) {
#if defined(_MSC_VER)
						g_CpuInfo.m_LogicalCoreCount +=
							static_cast<uint32_t>(__popcnt64(proc.GroupMask[i].Mask));
#else
						g_CpuInfo.m_LogicalCoreCount +=
							static_cast<uint32_t>(__builtin_popcountll(proc.GroupMask[i].Mask));
#endif
					}
					break;
				}

			case RelationNumaNode:
				{
					const auto& numa = entry->NumaNode;
					if (numa.NodeNumber < CORIUM_MAX_NUMA)
						g_CpuInfo.m_NumaNodeMasks[numa.NodeNumber] = numa.GroupMask.Mask;
					g_CpuInfo.m_NumaNodeCount++;
					break;
				}

			case RelationCache:
				{
					const auto& cache = entry->Cache;
					if (cache.Level == 1) g_CpuInfo.m_L1CacheSize += cache.CacheSize;
					else if (cache.Level == 2) g_CpuInfo.m_L2CacheSize += cache.CacheSize;
					else if (cache.Level == 3) g_CpuInfo.m_L3CacheSize += cache.CacheSize;
					break;
				}

			default: break;
			}

			p += entry->Size;
		}

		free(buffer);

		// CACHE LINE SIZE + SIMD CAPABILITIES

		int cpuInfo[4] = {};
		__cpuid(cpuInfo, 1);

		g_CpuInfo.m_CacheLineSize = ((cpuInfo[1] >> 8) & 0xFF) * 8;

		const bool hasSSE = (cpuInfo[3] & (1 << 25)) != 0;
		const bool hasSSE41 = (cpuInfo[2] & (1 << 19)) != 0;
		const bool hasAVX = (cpuInfo[2] & (1 << 28)) != 0;

		uint64_t xcr0 = 0;
#ifdef _XCR_XFEATURE_ENABLED_MASK
		xcr0 = _xgetbv(_XCR_XFEATURE_ENABLED_MASK);
#else
		xcr0 = _xgetbv(0);
#endif

		const bool avxOs = (xcr0 & 0x6) == 0x6;

		int cpuInfoEx[4] = {};
		__cpuidex(cpuInfoEx, 7, 0);

		const bool hasAVX2 = (cpuInfoEx[1] & (1 << 5)) != 0;
		const bool hasAVX512 = (cpuInfoEx[1] & (1 << 16)) != 0 && ((xcr0 & 0xE0) == 0xE0);

		uint8_t caps = 0;
		if (hasSSE)              caps |= (1 << 0);
		if (hasSSE41)            caps |= (1 << 1);
		if (hasAVX && avxOs)    caps |= (1 << 2);
		if (hasAVX2 && avxOs)    caps |= (1 << 3);
		if (hasAVX512)           caps |= (1 << 4);

		g_VectorizeCapabilities.m_VectorCapabilities = caps;

		// OS VERSION (RtlGetVersion — bypasses compatibility shim)

		typedef LONG(WINAPI* RtlGetVersionPtr)(PRTL_OSVERSIONINFOW);

		HMODULE hMod = GetModuleHandleA("ntdll.dll");
		if (hMod) {
			union {
				FARPROC        raw;
				RtlGetVersionPtr typed;
			} fn_cast;

			fn_cast.raw = GetProcAddress(hMod, "RtlGetVersion");

			if (fn_cast.typed) {
				RTL_OSVERSIONINFOW ver = {};
				ver.dwOSVersionInfoSize = sizeof(ver);

				if (fn_cast.typed(&ver) == 0) {
					g_OsInfo.m_MajorVersion = ver.dwMajorVersion;
					g_OsInfo.m_MinorVersion = ver.dwMinorVersion;
					g_OsInfo.m_BuildNumber = ver.dwBuildNumber;
				}
			}
		}

		static char s_ProductName[64] = "Windows";

		if (g_OsInfo.m_MajorVersion == 10) {
			if (g_OsInfo.m_BuildNumber >= 22000)
				strcpy_s(s_ProductName, "Windows 11");
			else
				strcpy_s(s_ProductName, "Windows 10");
		}

		g_OsInfo.m_ProductName = s_ProductName;

#elif defined(__linux__)

		// Linux fallback — CPUID still works, topology via /proc/cpuinfo

		int cpuInfo[4] = {};
		__cpuid(1, cpuInfo[0], cpuInfo[1], cpuInfo[2], cpuInfo[3]);

		g_CpuInfo.m_CacheLineSize = ((cpuInfo[1] >> 8) & 0xFF) * 8;

		const bool hasSSE = (cpuInfo[3] & (1 << 25)) != 0;
		const bool hasSSE41 = (cpuInfo[2] & (1 << 19)) != 0;
		const bool hasAVX = (cpuInfo[2] & (1 << 28)) != 0;

		int cpuInfoEx[4] = {};
		__cpuid_count(7, 0, cpuInfoEx[0], cpuInfoEx[1], cpuInfoEx[2], cpuInfoEx[3]);

		const bool hasAVX2 = (cpuInfoEx[1] & (1 << 5)) != 0;
		const bool hasAVX512 = (cpuInfoEx[1] & (1 << 16)) != 0;

		uint8_t caps = 0;
		if (hasSSE)    caps |= (1 << 0);
		if (hasSSE41)  caps |= (1 << 1);
		if (hasAVX)    caps |= (1 << 2);
		if (hasAVX2)   caps |= (1 << 3);
		if (hasAVX512) caps |= (1 << 4);

		g_VectorizeCapabilities.m_VectorCapabilities = caps;

		// OS info — sysname from uname or /etc/os-release not implemented here
		static char s_ProductName[64] = "Linux";
		g_OsInfo.m_ProductName = s_ProductName;
#endif

		g_IsInitialized = true;
	}

	const CpuInfo& EnvironmentProbe::getCpuInfo() {
		CORIUM_ASSERT(g_IsInitialized);
		return g_CpuInfo;
	}

	const VectorizeCapabilities& EnvironmentProbe::getVectorizeCapabilities() {
		CORIUM_ASSERT(g_IsInitialized);
		return g_VectorizeCapabilities;
	}

	const OsInfo& EnvironmentProbe::getOsInfo() {
		CORIUM_ASSERT(g_IsInitialized);
		return g_OsInfo;
	}

	// -------------------------------------------------------------------------
	// CoriumProcess
	// -------------------------------------------------------------------------

	uint32_t CoriumProcess::getCurrentProcessId() {
#ifdef _WIN32
		return GetCurrentProcessId();
#else
		return static_cast<uint32_t>(getpid());
#endif
	}

	const char* CoriumProcess::getExecutablePath() {
#ifdef _WIN32
		static char s_Path[MAX_PATH];
		GetModuleFileNameA(nullptr, s_Path, MAX_PATH);
		return s_Path;
#else
		static char s_Path[4096];
		ssize_t len = readlink("/proc/self/exe", s_Path, sizeof(s_Path) - 1);
		s_Path[len > 0 ? len : 0] = '\0';
		return s_Path;
#endif
	}

	const char* CoriumProcess::getWorkingDirectory() {
#ifdef _WIN32
		static char s_Dir[MAX_PATH];
		GetCurrentDirectoryA(MAX_PATH, s_Dir);
		return s_Dir;
#else
		static char s_Dir[4096];
		return getcwd(s_Dir, sizeof(s_Dir));
#endif
	}

	size_t CoriumProcess::getEnvironmentVariable(const char* p_Name, char* p_Buffer, unsigned long v_BufferSize) {
#ifdef _WIN32
		return GetEnvironmentVariableA(p_Name, p_Buffer, v_BufferSize);
#else
		const char* val = getenv(p_Name);
		if (!val) return 0;
		size_t len = strlen(val);
		if (p_Buffer && v_BufferSize > len) {
			memcpy(p_Buffer, val, len + 1);
		}
		return len;
#endif
	}

	bool CoriumProcess::setEnvironmentVariable(const char* p_Name, const char* p_Value) {
#ifdef _WIN32
		return SetEnvironmentVariableA(p_Name, p_Value) != 0;
#else
		return setenv(p_Name, p_Value, 1) == 0;
#endif
	}

	// -------------------------------------------------------------------------
	// CoriumTermination
	// -------------------------------------------------------------------------

	CORIUM_NORETURN void CoriumTermination::terminate(const char* p_Reason, const char* p_FileName, int v_Line) {
		CORIUM_UNUSED(p_Reason);
		CORIUM_UNUSED(p_FileName);
		CORIUM_UNUSED(v_Line);
		//TODO
#ifdef _WIN32
		TerminateProcess(GetCurrentProcess(), EXIT_FAILURE);
#else
		_exit(EXIT_FAILURE);
#endif
		CORIUM_UNREACHABLE();
	}
}