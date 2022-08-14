#pragma once

#include "IKeeper.h"
#include <assert.h>

class IProcessable {
 public:
  virtual void Process() = 0;
};
