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
namespace Corium::IntermediateRepresentation {
	// My ass had an itch, so i added IR, deal with it bish!

	template <size_t Rank>
	struct CORIUM_RUNTIME_API TensorStrideHelper;

	// Rank 0 - scalar (no strides)
	template <>
	struct CORIUM_RUNTIME_API TensorStrideHelper<0> {
		static constexpr std::array<size_t, 0>
			compute(const std::array<size_t, 0>&) {
			return {};
		}
	};

	// Rank 1 - vector
	template <>
	struct CORIUM_RUNTIME_API TensorStrideHelper<1> {
		static constexpr std::array<size_t, 1>
			compute(const std::array<size_t, 1>& extents, size_t elementSize) {
			return { elementSize };
		}
	};

	// Rank 2 - matrix
	template <>
	struct CORIUM_RUNTIME_API TensorStrideHelper<2> {
		static constexpr std::array<size_t, 2>
			compute(const std::array<size_t, 2>& extents, size_t elementSize) {
			return {
				extents[1] * elementSize,
				elementSize
			};
		}
	};

	// Rank 3 - grid
	template <>
	struct CORIUM_RUNTIME_API TensorStrideHelper<3> {
		static constexpr std::array<size_t, 3>
			compute(const std::array<size_t, 3>& extents, size_t elementSize) {
			return {
				extents[1] * extents[2] * elementSize,
				extents[2] * elementSize,
				elementSize
			};
		}
	};

	template <typename T, size_t... Extents>
	consteval uint64_t computeTypeHash() {
		constexpr std::string_view sig =
#if defined(_MSC_VER)
			__FUNCSIG__;
#else
			__PRETTY_FUNCTION__;
#endif
		uint64_t hash = 14695981039346656037ULL;
		for (char c : sig) {
			hash ^= static_cast<uint64_t>(c);
			hash *= 1099511628211ULL;
		}
		return hash;
	}

	template <typename T, size_t... Extents>
	struct CORIUM_RUNTIME_API TensorParameter {
		using DataType = T;
		static constexpr size_t ElementSize = sizeof(DataType);
		static constexpr size_t ElementAlignment = alignof(DataType);
		static constexpr size_t Rank = sizeof...(Extents);
		CORIUM_STATIC_ASSERT(Rank <= 3, "TensorParameter supports up to 3 dimensions");

		static constexpr std::array<size_t, Rank> ExtentArray = []() consteval {
			if constexpr (Rank == 0) return { };
			else return { Extents..., };
			}();

		static constexpr std::array<size_t, Rank> Strides =
			TensorStrideHelper<Rank>::compute(ExtentArray, ElementSize);

		static constexpr size_t PayloadSize = []() consteval {
			if constexpr (Rank == 0) return ElementSize;
			else return ElementSize * (Extents * ...);
			}();

		static constexpr size_t TypeHash = computeTypeHash<T, Extents...>();
	};

	template <typename T>
	using ScalarParameter = TensorParameter<T>;

	template <typename T, size_t N>
	using VectorParameter = TensorParameter<T, N>;

	template <typename T, size_t M, size_t N>
	using MatrixParameter = TensorParameter<T, M, N>;

	template <typename T, size_t X, size_t Y, size_t Z>
	using GridParameter = TensorParameter<T, X, Y, Z>;


	// Use these for actual params

	template <typename T, size_t Pos>
	using Scalar = ScalarParameter<T>; 

	template <typename T, size_t N, size_t Pos>
	using Vector = VectorParameter<T, N>;

	template <typename T, size_t M, size_t N, size_t Pos>
	using Matrix = MatrixParameter<T, M, N>;

	template <typename T, size_t X, size_t Y, size_t Z, size_t Pos>
	using Grid = GridParameter<T, X, Y, Z>;
}
