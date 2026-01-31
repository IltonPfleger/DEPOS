#pragma once

#include <utils/Lists.hpp>

class Observed;
class Observer;

class Observed {
    friend Observer;

  public:
    Observed() = default;
    void attach(Observer *o);
    void detach(Observer *o);
    virtual void notify();

  private:
    using Link = Node<Observer *>;
    LIFO<Link> m_observers;
};

class Observer {
    friend Observed;

  public:
    Observer() : m_link(this) {}
    virtual void update() = 0;

  private:
    Observed::Link m_link;
};

inline void Observed::attach(Observer *o) { m_observers.insert(&o->m_link); }

inline void Observed::detach(Observer *o) { m_observers.remove(&o->m_link); }

inline void Observed::notify() {
    for (Link *l = m_observers.head(); l; l = l->next())
        l->value()->update();
}
