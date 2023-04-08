#pragma once

#include <stdint.h>
#include "ophelper-config.h"

namespace ophelper {

#if !defined (SYS_MEMORY_LE) && !defined(SYS_MEMORY_BE)
#define SYS_MEMORY_LE
#endif

#ifndef HWREGH
#define HWREGH(regAddress) (*(uint16_t*) (regAddress))
#endif

#ifndef SWAP_16BIT_U_VALUE
#define SWAP_16BIT_U_VALUE(data)  ((data << 8) & 0xff00) | ((data >> 8) & 0x00ff)
#endif

#ifndef SWAP_32BIT_U_VALUE
#define SWAP_32BIT_U_VALUE(data)  (\
    (((data) & 0xFF000000) >> 24) \
    | (((data) & 0x00FF0000) >> 8) \
    | (((data) & 0x0000FF00) << 8) \
    | (((data) & 0x000000FF) << 24))
#endif


#ifdef SYS_MEMORY_LE

#define __SWAP_U16_LE__(data) (data)
#define __SWAP_U32_LE__(data) (data)
#define __SWAP_U16_BE__(data) (SWAP_16BIT_U_VALUE(data))
#define __SWAP_U32_BE__(data) (SWAP_32BIT_U_VALUE(data))

#else // big endian

#define __SWAP_U16_LE__(data) (SWAP_16BIT_U_VALUE(data))
#define __SWAP_U32_LE__(data) (SWAP_32BIT_U_VALUE(data))
#define __SWAP_U16_BE__(data) (data)
#define __SWAP_U32_BE__(data) (data)

#endif // #ifdef SYS_MEMORY_LE


constexpr uint16_t to_be_u16(const uint16_t value) {
  return __SWAP_U16_BE__(value);
}

constexpr uint32_t to_be_u32(const uint32_t value) {
  return __SWAP_U32_BE__(value);
}

constexpr uint16_t to_le_u16(const uint16_t value) {
  return __SWAP_U16_LE__(value);
}

constexpr uint32_t to_le_u32(const uint32_t value) {
  return __SWAP_U32_LE__(value);
}

constexpr uint16_t from_be_u16(const uint16_t value) {
  return to_be_u16(value);
}

constexpr uint32_t from_be_u32(const uint32_t value) {
  return to_be_u32(value);
}

constexpr uint16_t from_le_u16(const uint16_t value) {
  return to_le_u16(value);
}

constexpr uint32_t from_le_u32(const uint32_t value) {
  return to_le_u32(value);
}

template<int32_t Begin, int32_t End, class T>
constexpr bool is_in_range(T value) {
  static_assert(Begin <= End, "Range begin must be less than end");
  return (static_cast<int32_t>(value) >= Begin) && (static_cast<int32_t>(value) <= End);
}

template<int32_t Begin, int32_t End, class T>
constexpr bool is_out_range(T value) {
  return !is_in_range<Begin, End>(value);
}


#undef __SWAP_U16_LE__
#undef __SWAP_U32_LE__
#undef __SWAP_U16_BE__
#undef __SWAP_U32_BE__

#undef SWAP_16BIT_U_VALUE
#undef SWAP_32BIT_U_VALUE

}
