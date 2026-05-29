#ifndef __DEPOS_FDT_NODE__
#define __DEPOS_FDT_NODE__

namespace DEPOS {

template <typename T = void> class FDT_Node {
  public:
    using Type = T;

    FDT_Node(const char *name)
        : name_(name) {}

    FDT_Node(const char *name, T &&data)
        : name_(name),
          data_(data) {}

    const char *name() { return name_; }

  private:
    const char *name_;
    typename Meta::IF<Meta::IsVoid<T>::Result, Meta::Empty, T>::Result data_;
};

} // namespace DEPOS

#endif
