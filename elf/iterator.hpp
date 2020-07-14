#pragma once

namespace elf {
    template <class T> class iterator {
        T* ptr;
    
    public:
        iterator() = default;
        iterator(T* ptr) : ptr(ptr) {}

        iterator& operator++() { ++ptr; return *this; }
        bool operator!=(const iterator& other) const { return ptr != other.ptr; }
        const T& operator*() const { return *ptr; }
    };
}