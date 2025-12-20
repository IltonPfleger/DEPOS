#pragma once

template <unsigned int Min, unsigned int Max> class DispatchTable {
    static_assert(Min <= Max);

  public:
    using Function = void (*)();

    DispatchTable() = default;

    void bind(unsigned int index, Function function) {
        if (index >= Min && index < Max) {
            m_table[index - Min] = function;
        }
    }

    void dispatch(unsigned int index) {
        if (index >= Min && index < Max) {
            // Console::println("\n%x\n", reinterpret_cast<void *>(m_table[index - Min]));
            m_table[index - Min]();
        }
    }

  private:
    Function m_table[Max - Min + 1];
};
