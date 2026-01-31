#pragma once

template <typename T, auto Get, auto Release> class Guard {

  public:
    Guard(T *t, bool get = true) : m_protected(t) {
        if (m_protected && get) (m_protected->*Get)();
    }

    ~Guard() {
        if (m_protected) (m_protected->*Release)();
    }

    Guard(const Guard &) = delete;
    Guard &operator=(const Guard &) = delete;

    Guard(Guard &&other) : m_protected(other.m_protected) { other.m_protected = nullptr; }

    Guard &operator=(Guard &&other) {
        if (this != &other) {
            if (m_protected) (m_protected->*Release)();
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
