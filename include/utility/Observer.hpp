#pragma once

#include <utility/collections/UnorderedList.hpp>

namespace DEPOS {

template <typename... Args> class Observer;
template <typename... Args> class Observed;

template <typename... Args> class Observed {
    using Link = collections::Node<Observer<Args...> *>;
    friend class Observer<Args...>;

  public:
    Observed()
        : obervers_(),
          size_(0) {}

    void attach(Observer<Args...> *o) {
        obervers_.insert(&o->link_);
        size_++;
    }

    void detach(Observer<Args...> *o) {
        obervers_.remove(&o->link_);
        size_--;
    }

    uint32_t size() { return size_; }

    void notify(Args... args) {
        for (Link *l = obervers_.head(); l; l = l->next) {
            l->value()->update(args...);
        }
    }

  private:
    collections::UnorderedList<Link> obervers_;
    uint32_t size_;
};

template <typename... Args> class Observer {
    friend class Observed<Args...>;

  public:
    Observer()
        : link_(this) {}

    virtual ~Observer() = default;

    virtual void update(Args... args) = 0;

  private:
    typename Observed<Args...>::Link link_;
};

} // namespace DEPOS
