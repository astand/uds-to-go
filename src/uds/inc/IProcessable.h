#pragma once

#include "IKeeper.h"

class IProcessable {
 public:
  virtual void Process() = 0;

 protected:
  /**
   * @brief default constructor with implementation
   * */
  IProcessable() {}

  /**
   * @brief constructor which can register itself to IKeeper typed
   * by IProcessable
   * */
  IProcessable(IKeeper<IProcessable> k) {

    if (k.Add(this) != IKeeper<IProcessable>::OK)
      for (;;);

  }
};
