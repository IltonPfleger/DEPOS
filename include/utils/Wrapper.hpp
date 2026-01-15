template <typename T>
concept WrapperInterface = requires { T::init(); };

template <WrapperInterface... Tn> class Wrapper {
  public:
    static void init() { (Tn::init(), ...); }
};
