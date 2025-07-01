export module Meta;

export namespace Meta {
    template <bool B, typename True, typename False>
    struct TypeSelector {
        using Type = True;
    };

    template <typename True, typename False>
    struct TypeSelector<false, True, False> {
        using Type = False;
    };

    template <bool Cond, typename T = void>
    struct IF {};

    template <typename T>
    struct IF<true, T> {
        using Type = T;
    };

}  // namespace Meta
