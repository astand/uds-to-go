#pragma once

#include <utility>
#include <string.h>
#include <vector>

using onepair = std::pair<std::string, std::string>;
using argsret = std::vector<onepair>;

argsret collectargs(int argc, char** argv) {

  argsret ret {0};
  onepair param;

  for (int i = 0; i < argc; i++) {
    // key found (must start with '-' (e.g. '-dbc'))
    if (argv[i][0] == '-') {
      param.first = std::string(argv[i]);
      param.second.clear();

      if ((i + 1) < argc && argv[i + 1][0] != '-') {
        // key param
        param.second = std::string(argv[i + 1]);
        // unlooped i incremention
        ++i;
      }

      ret.push_back(param);
    }
  }

  return ret;
}
