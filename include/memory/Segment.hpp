#pragma once
#include <memory/Memory.hpp>

class Segment {
  public:
    Segment(uintptr_t addr, size_t size, bool owned = true)
        : addr_(reinterpret_cast<char *>(addr)), size_(size), owned_(owned) {}

    Segment(size_t size)
        : Segment(reinterpret_cast<uintptr_t>(Memory::kmalloc(size)), size) {}

    ~Segment() {
        if (owned_)
            Memory::kfree(addr_, size_);
    }

    char *base() const { return addr_; }
    char *end() const { return addr_ + size_; }
    size_t size() const { return size_; }

  private:
    char *const addr_;
    const size_t size_;
    const bool owned_;
};
