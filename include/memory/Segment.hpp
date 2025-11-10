#pragma once
#include <IO/Debug.hpp>
#include <memory/Memory.hpp>

class Segment {
  public:
    Segment(void *addr, size_t size) : addr_(static_cast<char *>(addr)), size_(size) {}
    Segment(size_t size) : Segment(Memory::kmalloc(size), size) {}

    size_t size() const { return size_; }
    char *base() const { return addr_; }
    char *end() const { return addr_ + size_; }

  private:
    char *const addr_;
    const size_t size_;
};
