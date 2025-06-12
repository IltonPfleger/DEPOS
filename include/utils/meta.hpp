#ifndef IF_H
#define IF_H

namespace Meta {

template <bool V, typename True, typename False>
struct TypeSelector {
    using Type = True;
};

template <typename True, typename False>
struct TypeSelector<false, True, False> {
    using Type = False;
};

}  // namespace Meta

#endif
