#pragma once
#include <memory/Memory.hpp>

class Segment {
   public:
    Segment(size_t size) : addr_(static_cast<char*>(Memory::kmalloc(size))), size_(size) {}

    size_t size() const { return size_; }
    char* base() const { return addr_; }
    char* end() const { return addr_ + size_; }

   private:
    char* const addr_;
    const size_t size_;
};
