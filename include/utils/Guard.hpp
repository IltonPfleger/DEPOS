#pragma once

template <typename T, auto GET = &T::get, auto RELEASE = &T::release> class Guard {

  public:
    Guard(T *t, bool get = true) : m_protected(t) {
        if (m_protected && get) (m_protected->*GET)();
    }

    ~Guard() {
        if (m_protected) (m_protected->*RELEASE)();
    }

    Guard(const Guard &) = delete;
    Guard &operator=(const Guard &) = delete;

    Guard(Guard &&other) : m_protected(other.m_protected) { other.m_protected = nullptr; }

    Guard &operator=(Guard &&other) {
        if (this != &other) {
            if (m_protected) (m_protected->*RELEASE)();
            m_protected = other.m_protected;
            other.m_protected = nullptr;
        }
        return *this;
    }

    T &operator*() const { return *m_protected; }
    T *operator->() const { return m_protected; }
    T &operator[](size_t i) const { return m_protected[i]; }

  private:
    T *m_protected;
};
