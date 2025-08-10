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
#include "ThreadPlatform.h"

namespace Blaze::Factory {

	class BLAZE IThreadFactory {
	public:
		virtual ~IThreadFactory() = default;

		IThreadFactory() = default;
		IThreadFactory(const IThreadFactory&) = delete;
		IThreadFactory& operator=(const IThreadFactory&) = delete;

		IThreadFactory(IThreadFactory&&) noexcept = delete;
		IThreadFactory&& operator=(IThreadFactory&&) = delete;

		virtual Platform::ThreadHandle createThread(Platform::NativeThreadAttributes v_Attr,
			Platform::NativeThreadOptions v_Options) = 0;
	};
}
