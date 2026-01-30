#pragma once

class Linux {
  public:
    Linux(uintptr_t kernel, uintptr_t dtb) { reinterpret_cast<void (*)(unsigned int, uintptr_t)>(kernel)(CPU::id(), dtb); };
};
