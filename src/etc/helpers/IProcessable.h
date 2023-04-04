#pragma once

/// @brief Processable interface
class IProcessable {
 public:

  /// @brief Run process implementation
  virtual void Process() = 0;
};
