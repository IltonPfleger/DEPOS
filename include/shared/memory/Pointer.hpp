#ifndef __MEMORY_POINTER_HEADER
#define __MEMORY_POINTER_HEADER

#include <types.hpp>

namespace DEPOS {

template <typename T> class Pointer {
  public:
    constexpr Pointer()
        : m_pointer(nullptr) {}

    constexpr Pointer(decltype(nullptr))
        : m_pointer(nullptr) {}

    explicit Pointer(T *pointer)
        : m_pointer(pointer) {}

    ~Pointer() { reset(); }

    Pointer(const Pointer &)            = delete;
    Pointer &operator=(const Pointer &) = delete;

    Pointer(Pointer &&other) noexcept
        : m_pointer(other.release()) {}

    Pointer &operator=(Pointer &&other) noexcept {
        if (this != &other) {
            reset(other.release());
        }
        return *this;
    }

    [[nodiscard]] explicit operator bool() const { return m_pointer != nullptr; }

    [[nodiscard]] T &operator*() const { return *m_pointer; }

    [[nodiscard]] T *operator->() const { return m_pointer; }

    T *release() {
        T *old    = m_pointer;
        m_pointer = nullptr;
        return old;
    }

    void reset(T *pointer = nullptr) {
        T *old    = m_pointer;
        m_pointer = pointer;
        if (old) delete old;
    }

    template <typename... Args> [[nodiscard]] static Pointer<T> make(Args &&...args) {
        return Pointer<T>(new T(static_cast<Args &&>(args)...));
    }

  private:
    T *m_pointer;
};

} // namespace DEPOS

#endif
