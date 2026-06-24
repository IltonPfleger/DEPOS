#ifndef __QUARK_GUARD__
#define __QUARK_GUARD__

namespace QUARK {

template <typename T, auto Enter, auto Exit> class Guard;

template <typename T, void (T::*Enter)(), void (T::*Exit)()> class Guard<T, Enter, Exit> {
  public:
    Guard(T &object)
        : object_(object) {
        (object_.*Enter)();
    }

    ~Guard() { (object_.*Exit)(); }

  private:
    T &object_;
};

template <typename State, State (*Enter)(), void (*Exit)(State)> class Guard<void, Enter, Exit> {
  public:
    Guard()
        : state_(Enter()) {}
    ~Guard() { Exit(state_); }

  private:
    State state_;
};

} // namespace QUARK

#endif
