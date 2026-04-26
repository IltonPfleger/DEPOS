#ifndef __META_TYPE_LIST_HEADER__
#define __META_TYPE_LIST_HEADER__

#include <shared/meta/Tuple.hpp>

namespace DEPOS {

namespace meta {

template <typename... Ts> struct TypeList {
    static constexpr unsigned int Length = sizeof...(Ts);
    static constexpr bool Empty          = (Length == 0);
};

template <typename, unsigned int> struct GetFromTypeList;

template <typename Head, typename... Tail> struct GetFromTypeList<TypeList<Head, Tail...>, 0> {
    using Result = Head;
};

template <typename Head, typename... Tail, unsigned int Index> struct GetFromTypeList<TypeList<Head, Tail...>, Index> {
    using Result = typename GetFromTypeList<TypeList<Tail...>, Index - 1>::Result;
};

template <typename... Ts, typename Function> void forEach(TypeList<Ts...>, Function f) {
    (f.template operator()<Ts>(), ...);
}

} // namespace meta

} // namespace DEPOS

#endif
