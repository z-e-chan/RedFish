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
#include "allocator.h"

namespace rf
{
template <typename T>
class NonAllocatingList
{
public:
    NonAllocatingList(int maxNumObjects)
        : m_maxNumObjects(maxNumObjects)
    {
        m_data = Allocator::s_allocate(m_maxNumObjects * sizeof(T), "NonAllocatingList", alignof(T));
    }

    ~NonAllocatingList()
    {
        Allocator::s_deallocate(m_data);
        m_data = nullptr;
    }

    bool operator==(const NonAllocatingList& list) const
    {
        if (m_maxNumObjects != list.m_maxNumObjects)
        {
            return false;
        }

        if (m_numObjects != list.m_numObjects)
        {
            return false;
        }

        return memcmp(m_data, list.m_data, m_numObjects) == 0;
    }

    bool operator!=(const NonAllocatingList& list) const
    {
        const bool isSame = *this == list;
        return !isSame;
    }

    typedef T* iterator;
    typedef const T* const_iterator;

    iterator begin()
    {
        T* data = reinterpret_cast<T*>(m_data);
        return &data[0];
    }

    const_iterator begin() const
    {
        const T* data = reinterpret_cast<const T*>(m_data);
        return &data[0];
    }

    iterator end()
    {
        T* data = reinterpret_cast<T*>(m_data);
        return &data[m_numObjects];
    }

    const_iterator end() const
    {
        const T* data = reinterpret_cast<const T*>(m_data);
        return &data[m_numObjects];
    }

    int GetSize() const
    {
        return m_numObjects;
    }

    int GetMaxSize() const
    {
        return m_maxNumObjects;
    }

    bool Append(const T& value)
    {
        if (m_numObjects >= m_maxNumObjects)
        {
            return false;
        }

        T* data = reinterpret_cast<T*>(m_data);
        data[m_numObjects++] = value;
        return true;
    }

    bool Insert(int index, const T& value)
    {
        if (m_numObjects >= m_maxNumObjects)
        {
            return false;
        }

        const int objectsToMove = m_numObjects - index;
        ++m_numObjects;
        const int bytesToMove = objectsToMove * sizeof(T);
        T* data = reinterpret_cast<T*>(m_data);
        memcpy(data + index + 1, data + index, bytesToMove);
        data[index] = value;
        return true;
    }

    const T& Get(int index, bool* outSuccess = nullptr) const
    {
        if (index < 0)
        {
            if (outSuccess)
            {
                *outSuccess = false;
            }

            const T* data = reinterpret_cast<const T*>(m_data);
            return data[0];
        }

        if (index >= m_numObjects)
        {
            if (outSuccess)
            {
                *outSuccess = false;
            }

            const T* data = reinterpret_cast<const T*>(m_data);
            return data[0];
        }

        if (outSuccess)
        {
            *outSuccess = true;
        }

        const T* data = reinterpret_cast<const T*>(m_data);
        return data[index];
    }

    bool Contains(const T& value) const
    {
        const T* data = reinterpret_cast<const T*>(m_data);
        for (int i = 0; i < m_numObjects; ++i)
        {
            if (value == data[i])
            {
                return true;
            }
        }

        return false;
    }

    void Erase(int index)
    {
        if (m_numObjects == 0)
        {
            return;
        }

        T* data = reinterpret_cast<T*>(m_data);
        data[index] = data[m_numObjects - 1];
        --m_numObjects;
    }

    void EraseOrdered(int index)
    {
        if (m_numObjects == 0)
        {
            return;
        }

        const int objectsToMove = m_numObjects - index - 1;
        --m_numObjects;
        const int bytesToMove = objectsToMove * sizeof(T);
        T* data = reinterpret_cast<T*>(m_data);
        memcpy(data + index, data + index + 1, bytesToMove);
    }

    void Clear()
    {
        m_numObjects = 0;
    }

private:
    void* m_data = nullptr;
    int m_maxNumObjects = 0;
    int m_numObjects = 0;
};
}  // namespace rf