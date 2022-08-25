#pragma once

#include <stddef.h>
#include <utility>
#include <string>

bool set_byte(char c, uint8_t& byte, bool is_high);

void try_to_set_param(const std::pair<std::string, std::string>& pair, uint32_t& vset);
