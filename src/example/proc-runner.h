#pragma once

#include <helpers/IProcessable.h>
#include <helpers/IKeeper.h>

template<size_t N>
class ProcRunner : public IKeeper<IProcessable> {
 public:
  ProcRunner() : IKeeper<IProcessable>(__raw__, N) {
    assert(N != 0);
  }

  void RunAllProcess() {
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