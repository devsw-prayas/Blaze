#pragma once
#include "Corium.h"
#include "CoriumCompiler.h"

namespace Corium::Core::Chrono {

	struct CORIUM_ALIGNAS(8) Interval final {
		int64_t m_Nanoseconds{ 0 };

		constexpr Interval() noexcept = default;
		constexpr explicit Interval(int64_t v_Ns) noexcept : m_Nanoseconds(v_Ns) {}

		CORIUM_NODISCARD constexpr Interval operator+(const Interval& ro_Other) const noexcept { return Interval{ m_Nanoseconds + ro_Other.m_Nanoseconds }; }
		CORIUM_NODISCARD constexpr Interval operator-(const Interval& ro_Other) const noexcept { return Interval{ m_Nanoseconds - ro_Other.m_Nanoseconds }; }
		CORIUM_NODISCARD constexpr Interval operator*(int64_t v_Scale)          const noexcept { return Interval{ m_Nanoseconds * v_Scale }; }
		CORIUM_NODISCARD constexpr Interval operator/(int64_t v_Divisor)        const noexcept { return Interval{ m_Nanoseconds / v_Divisor }; }

		CORIUM_NODISCARD constexpr bool operator==(const Interval& ro_Other) const noexcept { return m_Nanoseconds == ro_Other.m_Nanoseconds; }
		CORIUM_NODISCARD constexpr bool operator< (const Interval& ro_Other) const noexcept { return m_Nanoseconds <  ro_Other.m_Nanoseconds; }
		CORIUM_NODISCARD constexpr bool operator> (const Interval& ro_Other) const noexcept { return m_Nanoseconds >  ro_Other.m_Nanoseconds; }
		CORIUM_NODISCARD constexpr bool operator<=(const Interval& ro_Other) const noexcept { return m_Nanoseconds <= ro_Other.m_Nanoseconds; }
		CORIUM_NODISCARD constexpr bool operator>=(const Interval& ro_Other) const noexcept { return m_Nanoseconds >= ro_Other.m_Nanoseconds; }

		CORIUM_NODISCARD constexpr int64_t toNanoseconds()  const noexcept { return m_Nanoseconds; }
		CORIUM_NODISCARD constexpr int64_t toMicroseconds() const noexcept { return m_Nanoseconds / 1'000LL; }
		CORIUM_NODISCARD constexpr int64_t toMilliseconds() const noexcept { return m_Nanoseconds / 1'000'000LL; }
		CORIUM_NODISCARD constexpr int64_t toSeconds()      const noexcept { return m_Nanoseconds / 1'000'000'000LL; }
	};

	CORIUM_NODISCARD constexpr Interval nanoseconds(int64_t v_Ns) noexcept { return Interval{ v_Ns }; }
	CORIUM_NODISCARD constexpr Interval microseconds(int64_t v_Us) noexcept { return Interval{ v_Us * 1'000LL }; }
	CORIUM_NODISCARD constexpr Interval milliseconds(int64_t v_Ms) noexcept { return Interval{ v_Ms * 1'000'000LL }; }
	CORIUM_NODISCARD constexpr Interval seconds(int64_t v_S)       noexcept { return Interval{ v_S  * 1'000'000'000LL }; }
	CORIUM_NODISCARD constexpr Interval minutes(int64_t v_M)       noexcept { return Interval{ v_M  * 60'000'000'000LL }; }

	struct CORIUM_ALIGNAS(8) Instant final {
		int64_t m_Nanoseconds{ 0 };

		constexpr Instant() noexcept = default;
		constexpr explicit Instant(int64_t v_Ns) noexcept : m_Nanoseconds(v_Ns) {}

		CORIUM_NODISCARD CORIUM_RUNTIME_API bool     isExpired()             const noexcept;
		CORIUM_NODISCARD CORIUM_RUNTIME_API Interval remaining()             const noexcept;
		CORIUM_NODISCARD CORIUM_RUNTIME_API int64_t  remainingMilliseconds() const noexcept;
	};

	class CORIUM_RUNTIME_API MonotonicClock final {
	public:
		CORIUM_NODISCARD static Instant now() noexcept;
	};

	CORIUM_NODISCARD CORIUM_RUNTIME_API Instant until(Interval v_Interval) noexcept;

	namespace Literals {
		CORIUM_NODISCARD constexpr Interval operator""_ns (unsigned long long v_Val) noexcept { return Interval{ static_cast<int64_t>(v_Val) }; }
		CORIUM_NODISCARD constexpr Interval operator""_us (unsigned long long v_Val) noexcept { return Interval{ static_cast<int64_t>(v_Val) * 1'000LL }; }
		CORIUM_NODISCARD constexpr Interval operator""_ms (unsigned long long v_Val) noexcept { return Interval{ static_cast<int64_t>(v_Val) * 1'000'000LL }; }
		CORIUM_NODISCARD constexpr Interval operator""_s  (unsigned long long v_Val) noexcept { return Interval{ static_cast<int64_t>(v_Val) * 1'000'000'000LL }; }
		CORIUM_NODISCARD constexpr Interval operator""_min(unsigned long long v_Val) noexcept { return Interval{ static_cast<int64_t>(v_Val) * 60'000'000'000LL }; }
	}
}
