template <typename A>
concept Allocator = requires(A allocator, unsigned long size, void *addr) {
    { allocator.insert(addr, size) };
    { allocator.remove(size) };
};
