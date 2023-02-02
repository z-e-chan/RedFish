// MIT License

// Copyright (c) 2023 Zach Chan

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#pragma once
#include <utility>

namespace rf
{
using AllocateCallback = void* (*)(size_t numBytes, const char* name, int alignment);
using DeallocateCallback = void (*)(void* data);

class Allocator
{
public:
    static void SetCallbacks(AllocateCallback allocate, DeallocateCallback deallocate);

    template <class T, typename... Args>
    static T* AllocateAligned(const char* name, int alignment, Args&&... args)
    {
        const int size = sizeof(T);
        void* data = s_allocate(size, name, alignment);
        return new (data) T(std::forward<Args>(args)...);
    }

    template <class T, typename... Args>
    static T* Allocate(const char* name, Args&&... args)
    {
        const int size = sizeof(T);
        void* data = s_allocate(size, name, alignof(T));
        return new (data) T(std::forward<Args>(args)...);
    }

    template <class T>
    static T* AllocateBytes(const char* name, int size)
    {
        void* data = s_allocate(size, name, alignof(T));
        return static_cast<T*>(data);
    }

    template <class T, typename... Args>
    static T* AllocateArray(const char* name, int num, Args&&... args)
    {
        const int size = sizeof(T) * num;
        T* data = static_cast<T*>(s_allocate(size, name, alignof(T)));
        for (int i = 0; i < num; ++i)
        {
            new (data + i) T(std::forward<Args>(args)...);
        }
        return data;
    }

    template <class T>
    static void Deallocate(T** data)
    {
        if (*data)
        {
            (*data)->~T();
            s_deallocate(*data);
            *data = nullptr;
        }
    }

    template <class T>
    static void DeallocateBytes(T** data)
    {
        if (*data)
        {
            s_deallocate(*data);
            *data = nullptr;
        }
    }

    template <class T>
    static void DeallocateArray(T** data, int num)
    {
        if (*data)
        {
            T* toDelete = nullptr;
            for (int i = 0; i < num; ++i)
            {
                toDelete = (*data) + i;
                toDelete->~T();
            }
            s_deallocate(*data);
            *data = nullptr;
        }
    }

    static AllocateCallback s_allocate;
    static DeallocateCallback s_deallocate;
};

extern Allocator g_allocator;
}  // namespace rf
