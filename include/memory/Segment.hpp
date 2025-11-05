#pragma once
#include <IO/Console.hpp>
#include <memory/Memory.hpp>

class Segment {
   public:
    Segment(size_t size) : addr_(static_cast<char*>(Memory::kmalloc(size))), size_(size) {}

   private:
    const char* addr_;
    const size_t size_;
};
