#include "app-helper.h"
#include <iostream>


bool set_byte(char c, uint8_t& byte, bool is_high)
{
  uint8_t value = 0u;

  if (c >= '0' && c <= '9')
  {
    value = c - '0';
  }
  else if (c >= 'a' && c <= 'f')
  {
    value = (c - 'a') + 10;
  }
  else if (c >= 'A' && c <= 'F')
  {
    value = (c - 'A') + 10;
  }
  else
  {
    return false;
  }

  if (is_high)
  {
    byte = value << 4;
  }
  else
  {
    byte |= value & 0x0fu;
  }

  return true;
}

void try_to_set_param(const std::pair<std::string, std::string>& pair, uint32_t& vset)
{
  uint32_t scaned = 0u;
  std::string frmt = "%u";

  if (pair.second.size() > 2 && pair.second.at(1) == 'x')
  {
    frmt = "%x";
  }

  if (sscanf(pair.second.c_str(), frmt.c_str(), &scaned) != 1)
  {
    std::cout << "Wrong value (" << pair.second << ") for parameter '" << pair.first << "'";
  }
  else
  {
    vset = scaned;
  }
}
