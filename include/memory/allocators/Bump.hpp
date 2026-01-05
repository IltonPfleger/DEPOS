#pragma once

class Bump {
  public:
    Bump(unsigned long start, unsigned long end) : current_(start), end_(end) {}

    void *remove(unsigned long size) { return reinterpret_cast<void *>(current_ += size); }

    void insert(void *, unsigned long);

  private:
    unsigned long current_;
    const unsigned long end_;
};
