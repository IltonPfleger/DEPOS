#pragma once

#include <utils/collections/LIFO.hpp>
#include <utils/collections/Node.hpp>

namespace DEPOS {

template <typename... Args> class Observer;
template <typename... Args> class Observed;

template <typename... Args> class Observed {
    using Link = collections::Node<Observer<Args...> *>;
    friend class Observer<Args...>;

  public:
    Observed() = default;

    void attach(Observer<Args...> *o) { m_observers.insert(&o->m_link); }

    void detach(Observer<Args...> *o) { m_observers.remove(&o->m_link); }

    void notify(Args... args) {
        for (Link *l = m_observers.head(); l; l = l->next()) {
            l->value()->update(args...);
        }
    }

  private:
    collections::LIFO<Link> m_observers;
};

template <typename... Args> class Observer {
    friend class Observed<Args...>;

  public:
    Observer()
        : m_link(this) {}
    virtual ~Observer() = default;

    virtual void update(Args... args) = 0;

  private:
    typename Observed<Args...>::Link m_link;
};

} // namespace DEPOS
