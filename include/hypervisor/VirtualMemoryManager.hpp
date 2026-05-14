#ifndef __DEPOS_VIRTUAL_MEMORY_MANAGER__
#define __DEPOS_VIRTUAL_MEMORY_MANAGER__

#include <memory/Chunk.hpp>

namespace DEPOS {

class VirtualMemoryManager {
  public:
    VirtualMemoryManager(const Chunk &&chunk)
        : chunk_(chunk) {}

    bool validate(const Chunk &&chunk) const { return chunk_.contains(chunk); }
    uintptr_t start() const { return chunk_.start(); }
    size_t size() const { return chunk_.size(); }

  private:
    Chunk chunk_;
};

} // namespace DEPOS

#endif
