#ifndef __meta_hpp
#define __meta_hpp

template <bool B, typename True, typename False>
struct TypeSelector {
    using Type = True;
};

template <typename True, typename False>
struct TypeSelector<false, True, False> {
    using Type = False;
};

// template <typename T, unsigned int N>
// struct Array {
//     T data[N];
// };

#endif
