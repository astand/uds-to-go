#pragma once

#include <stdint.h>
#include <array>

class SidConfig {
 public:

  constexpr SidConfig(bool isp, bool isu, bool isdef, bool issub, uint8_t minl) : flag(
      ((isp) ? (PHYSADDR) : (0u)) |
      ((isu) ? (BITSUP) : (0u)) |
      ((isdef) ? (DEFSUP) : (0u)) |
      ((issub) ? (NOSUBFUNC) : (0u)) |
      (minl & 0xffu)
    ) {
  }

  SidConfig() = default;

  void SetSID(uint8_t sid) {
    flag = (flag & (~0xffu)) | sid;
  }

  bool IsSupport() const {
    return ((flag & BITSUP) != 0u);
  }

  bool IsPhysicalOnly() const {
    return ((flag & PHYSADDR) != 0u);
  };

  bool IsMinLength(uint8_t len) const {
    return len >= (flag & 0xffu);
  }

  bool IsDefaultSess() const {
    return ((flag & DEFSUP) != 0u);
  }

 private:
  using flag_t = uint16_t;

  static constexpr flag_t BITSUP = (flag_t) (1u << 15u);
  static constexpr flag_t PHYSADDR = (flag_t) (1u << 14u);
  static constexpr flag_t DEFSUP = (flag_t) (1u << 13u);
  static constexpr flag_t NOSUBFUNC = (flag_t) (1 << 12u);

  flag_t flag;
};

enum class ddd
{
  d = 0,
  c = 0
};

class SidConfList {
 public:

  SidConfList() {
    for (size_t i = 0u; i < cfgArray.size(); i++) {
      cfgArray[i].SetSID(static_cast<uint8_t>(i));
    }

    c = ddd::c;
  }

  const SidConfig& ConfigAt(uint8_t index) {
    return cfgArray[index];
  }

 private:
  std::array<SidConfig, 0xff> cfgArray;
  ddd c;
};