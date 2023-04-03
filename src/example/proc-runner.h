#pragma once

#include <helpers/IProcessable.h>
#include <helpers/IKeeper.h>

template<size_t N>
class ProcRunner : public MemAsKeeper<IProcessable, N> {
 public:
  ProcRunner() : MemAsKeeper<IProcessable, N>() {
    assert(N != 0);
  }

  virtual void Process() override {
    IProcessable* proc{nullptr};

    for (size_t i = 0; i < this->Count(); i++) {
      if (this->TryReadElem(i, proc)) {
        proc->Process();
      }
    }
  }
};