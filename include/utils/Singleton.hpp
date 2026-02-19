#pragma once

template <typename T> class Singleton {
  protected:
    Singleton() = default;
    virtual ~Singleton() = default;

    static inline T *s_instance = nullptr;
    static inline unsigned int s_count = 0;

  public:
    static T *instance() {
        if (!s_instance && s_count == 0) {
            s_instance = new T();
        }
        s_count++;
        return s_instance;
    }

    static void release() {
        if (s_count > 0) {
            s_count--;
            if (s_count == 0) {
                delete s_instance;
                s_instance = nullptr;
            }
        }
    }

    Singleton(const Singleton &) = delete;
    void operator=(const Singleton &) = delete;
};
