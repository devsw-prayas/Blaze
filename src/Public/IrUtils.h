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

//Yes i added IR in this, deal with it!
namespace Corium::IntermediateRepresentation {
	// Stride helpers

	template<size_t Rank>
	struct CORIUM_RUNTIME_API TensorStrideHelper;

	template<>
	struct CORIUM_RUNTIME_API TensorStrideHelper<0> {
		static constexpr std::array<size_t, 0> compute(const std::array<size_t, 0>&, size_t) {
			return {};
		}
	};

	template<>
	struct CORIUM_RUNTIME_API TensorStrideHelper<1> {
		static constexpr std::array<size_t, 1> compute(const std::array<size_t, 1>&, size_t v_ElementSize) {
			return { v_ElementSize };
		}
	};

	template<>
	struct CORIUM_RUNTIME_API TensorStrideHelper<2> {
		static constexpr std::array<size_t, 2> compute(const std::array<size_t, 2>& extents, size_t v_ElementSize) {
			return { extents[1] * v_ElementSize, v_ElementSize };
		}
	};

	template<>
	struct CORIUM_RUNTIME_API TensorStrideHelper<3> {
		static constexpr std::array<size_t, 3> compute(const std::array<size_t, 3>& extents, size_t v_ElementSize) {
			return {
				extents[1] * extents[2] * v_ElementSize,
				extents[2] * v_ElementSize,
				v_ElementSize
			};
		}
	};

	// Type hash — FNV-1a over __FUNCSIG__ / __PRETTY_FUNCTION__
	// Encodes T and Extents only. Position is NOT part of TensorParameter.
	// Position is mixed in separately at parameterize/induce and get/put call sites.

	template<typename T, size_t... Extents>
	consteval uint64_t computeTypeHash() {
		constexpr std::string_view sig =
#if defined(_MSC_VER)
			__FUNCSIG__;
#else
			__PRETTY_FUNCTION__;
#endif
		uint64_t h = 14695981039346656037ULL;
		for (char c : sig) { h ^= static_cast<uint64_t>(c); h *= 1099511628211ULL; }
		return h;
	}

	// Mix a pack position into a raw TypeHash to produce a unique positioned key.
	// Same formula used on both sides: parameterize/induce expansion AND get/put call sites.
	consteval uint64_t mixPositionHash(uint64_t v_TypeHash, size_t v_Pos) {
		return v_TypeHash ^ (static_cast<uint64_t>(v_Pos) * 0x9e3779b97f4a7c15ULL);
	}

	// TensorParameter<T, Extents...>
	// Describes data shape only. No position — position is a task concern, not a type concern.

	template<typename T, size_t... Extents>
	struct CORIUM_RUNTIME_API TensorParameter {
		using DataType = T;

		static constexpr uint64_t TypeHash = computeTypeHash<T, Extents...>();
		static constexpr size_t   ElementSize = sizeof(T);
		static constexpr size_t   ElementAlignment = alignof(T);
		static constexpr size_t   Rank = sizeof...(Extents);

		CORIUM_STATIC_ASSERT(Rank <= 3, "TensorParameter supports up to rank 3");

		static constexpr std::array<size_t, Rank> ExtentArray = []() consteval {
			if constexpr (Rank == 0) return std::array<size_t, 0>{};
			else                     return std::array<size_t, Rank>{ Extents... };
			}();

		static constexpr std::array<size_t, Rank> Strides =
			TensorStrideHelper<Rank>::compute(ExtentArray, ElementSize);

		static constexpr size_t PayloadSize = []() consteval {
			if constexpr (Rank == 0) return ElementSize;
			else                     return ElementSize * (Extents * ...);
			}();
	};

	template<typename T>                               using ScalarParameter = TensorParameter<T>;
	template<typename T, size_t N>                     using VectorParameter = TensorParameter<T, N>;
	template<typename T, size_t M, size_t N>           using MatrixParameter = TensorParameter<T, M, N>;
	template<typename T, size_t X, size_t Y, size_t Z> using GridParameter = TensorParameter<T, X, Y, Z>;

	template<typename T>                               using Scalar = ScalarParameter<T>;
	template<typename T, size_t N>                     using Vector = VectorParameter<T, N>;
	template<typename T, size_t M, size_t N>           using Matrix = MatrixParameter<T, M, N>;
	template<typename T, size_t X, size_t Y, size_t Z> using Grid = GridParameter<T, X, Y, Z>;

	// In<Types...> / Out<Types...>
	//
	// Variadic parameter list wrappers for parameterize() and induce().
	// Each type at pack index N gets positioned hash = mixPositionHash(TypeHash, N).
	// get<T, N>() / put<T, N>() use the same formula to look up the correct slot.
	// Max 8 parameters per pack (matches ParamsPerTask).

	template<typename... Types>
	struct CORIUM_RUNTIME_API In {
		static constexpr size_t Count = sizeof...(Types);
		CORIUM_STATIC_ASSERT(Count >= 1 && Count <= 8, "In<> requires 1..8 parameters");

	private:
		template<size_t... Is>
		static consteval std::array<uint64_t, Count> buildHashes(std::index_sequence<Is...>) {
			return { mixPositionHash(Types::TypeHash, Is)... };
		}

	public:
		static constexpr std::array<uint64_t, Count> Hashes = buildHashes(std::make_index_sequence<Count>{});
		static constexpr std::array<size_t, Count> Sizes = { Types::PayloadSize... };
		static constexpr std::array<size_t, Count> Alignments = { Types::ElementAlignment... };
	};

	template<typename... Types>
	struct CORIUM_RUNTIME_API Out {
		static constexpr size_t Count = sizeof...(Types);
		CORIUM_STATIC_ASSERT(Count >= 1 && Count <= 8, "Out<> requires 1..8 parameters");

	private:
		template<size_t... Is>
		static consteval std::array<uint64_t, Count> buildHashes(std::index_sequence<Is...>) {
			return { mixPositionHash(Types::TypeHash, Is)... };
		}

	public:
		static constexpr std::array<uint64_t, Count> Hashes = buildHashes(std::make_index_sequence<Count>{});
		static constexpr std::array<size_t, Count> Sizes = { Types::PayloadSize... };
		static constexpr std::array<size_t, Count> Alignments = { Types::ElementAlignment... };
	};

	// MPH internals

	namespace Internal {
		consteval size_t mphCeilLog2(size_t n) {
			size_t p = 0, v = (n < 2 ? 1 : n - 1);
			while (v) { v >>= 1; ++p; }
			return p;
		}

		// Find odd multiplier d such that (hash * d) >> shift is injective over all N hashes.
		// Uses uint64_t bitmask — valid for TableSize <= 64 (N <= 64).
		template<size_t N>
		consteval uint64_t findMultiplier(const std::array<uint64_t, N>& v_Hashes, int v_Shift) {
			for (uint64_t d = 1; ; d += 2) {
				uint64_t seen = 0;
				bool ok = true;
				for (size_t i = 0; i < N; ++i) {
					const uint64_t bit = 1ull << ((v_Hashes[i] * d) >> v_Shift);
					if (seen & bit) { ok = false; break; }
					seen |= bit;
				}
				if (ok) return d;
			}
		}

		// slot_table[compressed_hash] = pack index of the matching parameter
		template<size_t N, size_t TableSize>
		consteval std::array<uint8_t, TableSize> buildSlotTable(
			const std::array<uint64_t, N>& v_Hashes,
			const std::array<size_t,   N>& v_SortedIdx,
			uint64_t v_D, int v_Shift) {
			std::array<uint8_t, TableSize> table{};
			for (size_t i = 0; i < N; ++i)
				table[(v_Hashes[i] * v_D) >> v_Shift] = static_cast<uint8_t>(v_SortedIdx[i]);
			return table;
		}

		// Optimal physical ordering via brute-force over all N! permutations (N <= 8 = max 40320).
		// Descending-alignment sort is a heuristic that can leave padding gaps that smaller items
		// could fill. This finds the true minimum-size packing at zero runtime cost (consteval).
		template<size_t N>
		consteval std::array<size_t, N> findOptimalOrder(
			const std::array<size_t, N>& v_Sizes,
			const std::array<size_t, N>& v_Aligns)
		{
			auto computeSize = [&](const std::array<size_t, N>& v_Order) -> size_t {
				size_t cur = 0;
				for (size_t i = 0; i < N; ++i) {
					const size_t s = v_Order[i];
					cur = (cur + v_Aligns[s] - 1) & ~(v_Aligns[s] - 1);
					cur += v_Sizes[s];
				}
				return cur;
			};

			std::array<size_t, N> perm{};
			for (size_t i = 0; i < N; ++i) perm[i] = i;

			std::array<size_t, N> best   = perm;
			size_t                bestSz = computeSize(perm);

			// Heap's algorithm — generates all N! permutations in-place
			std::array<size_t, N> c{};
			size_t i = 1;
			while (i < N) {
				if (c[i] < i) {
					const size_t j = (i % 2 == 0) ? 0 : c[i];
					const size_t tmp = perm[j]; perm[j] = perm[i]; perm[i] = tmp;
					const size_t sz = computeSize(perm);
					if (sz < bestSz) { bestSz = sz; best = perm; }
					++c[i];
					i = 1;
				} else {
					c[i] = 0;
					++i;
				}
			}
			return best;
		}

		// offset_table[slot] = byte offset in the sorted, packed buffer
		template<size_t N>
		consteval std::array<size_t, N> buildOffsetTable(
			const std::array<size_t, N>& v_Sizes,
			const std::array<size_t, N>& v_Aligns,
			const std::array<size_t, N>& v_SortedIdx) {
			std::array<size_t, N> offsets{};
			size_t cur = 0;
			for (size_t i = 0; i < N; ++i) {
				const size_t slot = v_SortedIdx[i];
				cur = (cur + v_Aligns[slot] - 1) & ~(v_Aligns[slot] - 1);
				offsets[slot] = cur;
				cur += v_Sizes[slot];
			}
			return offsets;
		}
	} // namespace Internal

	// MphTable — runtime MPH lookup structure.
	// Stored inline in TaskMemoryDesc. Populated by parameterize() / induce().

	struct CORIUM_RUNTIME_API MphTable final {
		uint64_t m_D = 0;
		size_t   m_OffsetTable[8] = {};  // offset 8, indexed by slot
		uint8_t  m_SlotTable[16] = {};  // offset 72, indexed by compressed hash
		int      m_Shift = 0;   // offset 88
		uint32_t m_SlotCount = 0;   // offset 92
		uint32_t m_TableSize = 0;   // offset 96
		// total 100 bytes, padded to 104

		CORIUM_NODISCARD CORIUM_FORCEINLINE size_t lookup(uint64_t v_Hash) const noexcept {
			return m_OffsetTable[m_SlotTable[(v_Hash * m_D) >> m_Shift]];
		}

		CORIUM_NODISCARD bool isValid() const noexcept { return m_D != 0; }
	};

	// TaskLayout<Pack>
	//
	// Compile-time MPH over an In<> or Out<> pack.
	// Sorts parameters by alignment descending (matching cook() behaviour).
	// buildMphTable() produces a runtime MphTable ready to embed in TaskMemoryDesc.
	// v_BaseOffset shifts all output offsets past the TaskError header (128 bytes).

	template<typename Pack>
	struct CORIUM_RUNTIME_API TaskLayout {
		static constexpr size_t N = Pack::Count;
		static constexpr size_t P = Internal::mphCeilLog2(N < 2 ? 2 : N);
		static constexpr size_t TableSize = 1ull << P;
		static constexpr int    Shift = static_cast<int>(64 - P);

		CORIUM_STATIC_ASSERT(N >= 1 && N <= 8, "TaskLayout requires 1..8 parameters");

	private:
		static constexpr std::array<size_t, N> s_SortedIdx =
			Internal::findOptimalOrder<N>(Pack::Sizes, Pack::Alignments);

		// Re-order hashes into sort order for MPH construction
		static constexpr std::array<uint64_t, N> s_SortedHashes = []() consteval {
			std::array<uint64_t, N> arr{};
			for (size_t i = 0; i < N; ++i) arr[i] = Pack::Hashes[s_SortedIdx[i]];
			return arr;
			}();

	public:
		static constexpr uint64_t D = Internal::findMultiplier(s_SortedHashes, Shift);

		// slot_table[(hash * D) >> Shift] = slot in sorted order
		static constexpr std::array<uint8_t, TableSize> SlotTable =
			Internal::buildSlotTable<N, TableSize>(s_SortedHashes, s_SortedIdx, D, Shift);

		// offset_table[original_pack_index] = byte offset in packed buffer
		static constexpr std::array<size_t, N> OffsetTable =
			Internal::buildOffsetTable<N>(Pack::Sizes, Pack::Alignments, s_SortedIdx);

		static constexpr size_t TotalSize = []() consteval {
			size_t cur = 0;
			for (size_t i = 0; i < N; ++i) {
				const size_t slot = s_SortedIdx[i];
				cur = (cur + Pack::Alignments[slot] - 1) & ~(Pack::Alignments[slot] - 1);
				cur += Pack::Sizes[slot];
			}
			return cur;
			}();

		static constexpr MphTable buildMphTable(size_t v_BaseOffset = 0) noexcept {
			MphTable t;
			t.m_D = D;
			t.m_Shift = Shift;
			t.m_SlotCount = static_cast<uint32_t>(N);
			t.m_TableSize = static_cast<uint32_t>(TableSize);
			for (size_t i = 0; i < TableSize; ++i) t.m_SlotTable[i] = SlotTable[i];
			for (size_t i = 0; i < N; ++i) t.m_OffsetTable[i] = OffsetTable[i] + v_BaseOffset;
			return t;
		}
	};
} // namespace Corium::IntermediateRepresentation
