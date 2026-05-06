#ifndef __DEPOS_RISCV64_TRAP_HANDLER_HEADER__
#define __DEPOS_RISCV64_TRAP_HANDLER_HEADER__

#include <architecture/riscv64/ExceptionHandler.hpp>

namespace DEPOS {

namespace riscv64 {

class TrapHandler {
  public:
    enum Type { Exception = 0, IRQ = 1 };

  private:
    using ID      = size_t;
    using Index   = uintmax_t;
    using Handler = void (*)(ID, Context *);

    static Index irq2index(ID id, Type type) { return id + (type * NumberOfExceptions); }

    static void dispatch(Context *c) {
        uintmax_t cause = c->cause;
        Type type       = (Type)(cause >> 63);
        ID id           = cause & ~(1ULL << 63);
        Index index     = irq2index(id, type);

        ERROR(index >= NumberOfTrapHandlers);
        ERROR(!s_handlers[index]);

        s_handlers[index](id, c);
    }

    template <typename Privilege, bool ChangeStack>
    __attribute__((naked, optimize("O0"), aligned(4))) static void entry() {
        using ContextHandler = ContextHandler<Privilege, ChangeStack>;
        dispatch(ContextHandler::push());
        ContextHandler::pop();
    };

  public:
    static void install(ID id, Handler handler, Type type = IRQ) {
        Index index = irq2index(id, type);
        ERROR(index >= NumberOfTrapHandlers, index);
        s_handlers[index] = handler;
    };

    template <typename Privilege, bool ChangeStack> static void init() {
        csrw<Privilege::TVEC>(entry<Privilege, ChangeStack>);
    }

    static void init() {
        for (int i = 0; i < 16; i++)
            install(i, ExceptionHandler::onTrap, Exception);
    }

  private:
    static constexpr int NumberOfExceptions                          = 16;
    static constexpr int NumberOfInterruptions                       = 16;
    static constexpr int NumberOfTrapHandlers                        = NumberOfExceptions + NumberOfInterruptions;
    static constinit inline Handler s_handlers[NumberOfTrapHandlers] = {nullptr};
};

} // namespace riscv64

} // namespace DEPOS

#endif
