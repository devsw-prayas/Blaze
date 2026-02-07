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
    struct TensorStrideHelper;

    // Rank 0 - scalar (no strides)
    template <>
    struct TensorStrideHelper<0> {
        static constexpr std::array<size_t, 0>
            compute(const std::array<size_t, 0>&) {
            return {};
        }
    };

    // Rank 1 - vector
    template <>
    struct TensorStrideHelper<1> {
        static constexpr std::array<size_t, 1>
            compute(const std::array<size_t, 1>& extents, size_t elementSize) {
            return { elementSize };
        }
    };

    // Rank 2 - matrix
    template <>
    struct TensorStrideHelper<2> {
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
    struct TensorStrideHelper<3> {
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
    struct TensorParameter {
        using DataType = T;
        static constexpr size_t Rank = sizeof...(Extents);
        static_assert(Rank <= 3, "TensorParameter supports up to 3 dimensions");

        static constexpr size_t ElementSize = sizeof(T);
        static constexpr size_t ElementAlignment = alignof(T);

        static constexpr std::array<size_t, Rank> ExtentArray = { Extents... };

        static constexpr std::array<size_t, Rank> Strides =
            TensorStrideHelper<Rank>::compute(ExtentArray, ElementSize);

        static constexpr size_t PayloadSize =
            ElementSize * (Extents * ...);
    };


	template <typename T>
    using ScalarParameter = TensorParameter<T>;

    template <typename T, size_t N>
    using VectorParameter = TensorParameter<T, N>;

    template <typename T, size_t M, size_t N>
    using MatrixParameter = TensorParameter<T, M, N>;

    template <typename T, size_t X, size_t Y, size_t Z>
    using GridParameter = TensorParameter<T, X, Y, Z>;

}
