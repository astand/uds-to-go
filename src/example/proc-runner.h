#pragma once

#include <helpers/IProcessable.h>
#include <helpers/IKeeper.h>

template<size_t N>
class ProcRunner : public AsKeeper<IProcessable> {
 public:
  ProcRunner() : AsKeeper<IProcessable>(__raw__, N) {
    assert(N != 0);
  }

  virtual void Process() override {
    IProcessable* proc{nullptr};

    for (size_t i = 0; i < Count(); i++) {
      if (Item(i, proc)) {
        proc->Process();
      }
    }
  }

 private:
  IProcessable* __raw__[N] {nullptr};
};