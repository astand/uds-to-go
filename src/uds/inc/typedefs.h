#pragma once

#include <stdint.h>

typedef uint32_t datasize_t;
typedef uint8_t enumbase_t;

template<class T>
static constexpr uint8_t to_byte(T t) {

  return static_cast<uint8_t>(t);
}

template<class T>
static constexpr T from_byte(uint8_t b) {

  return static_cast<T>(b);
}
