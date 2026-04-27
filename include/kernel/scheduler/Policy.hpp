#ifndef __SCHEDULER_POLICY_HEADER__
#define __SCHEDULER_POLICY_HEADER__

namespace DEPOS {

template <typename T> class Policy {
  public:
    Policy(T r, ...)
        : m_rank(r) {}
    operator T() const { return m_rank; }

  private:
    T m_rank;
};

} // namespace DEPOS

#endif
