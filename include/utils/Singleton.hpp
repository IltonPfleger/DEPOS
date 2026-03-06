#pragma once

template <typename T> class Singleton {
  public:
    template <typename... Args> static T *instance() {
        static T instance;
        return &instance;
    }

    Singleton()                             = delete;
    Singleton(const Singleton &)            = delete;
    Singleton &operator=(const Singleton &) = delete;
};
