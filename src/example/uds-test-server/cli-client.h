#pragma once

#include <assert.h>
#include <stdlib.h>
#include <stdint.h>
#include <iostream>
#include <vector>
#include <stack>
#include <mutex>

class Menu {
 public:
  Menu(const std::string& str) : text(str) {}

  std::string text;
  std::vector<uint8_t> cmd;

  Menu* SetNext(Menu* next) {
    assert(next != nullptr);
    this->next = next;
    return this->next;
  }

  Menu* SetDown(Menu* d) {
    assert(d != nullptr);
    this->down = d;
    return d;
  }

  Menu* GetNext() {
    return next;
  }

  bool IsNext() {
    return next != nullptr;
  }

  Menu* GetDown() {
    return down;
  }

  bool IsDown() {
    return down != nullptr;
  }


 private:

  Menu* next {nullptr};
  Menu* down {nullptr};

};

class CliMen {
 public:
  CliMen(Menu* m) : root(m) {
    assert(root != nullptr);
  }

 public:
  void Run() {
    Menu* item = root;
    Menu* start = root;

    while (true) {

      if (!deep.empty()) {
        start = deep.top();
      }
      else {
        start = root;
      }

      item = start;
      uint32_t count = 0;

      do {
        std::cout << ++count << " : " << item->text << std::endl;
        item = item->GetNext();
      }
      while (item != nullptr);

      std::string in;
      std::cout << "Input number > ";
      std::cin >> in;
      int32_t scaned = -1;

      if ((sscanf(in.c_str(), "%d", &scaned)) == 1) {
        // try to find item on current level
        if (scaned > 0) {
          item = start;

          while (--scaned && item != nullptr) {
            item = item->GetNext();
          }

          if (item != nullptr) {
            if (item->IsDown()) {
              // get down pointer
              item = item->GetDown();
              deep.push(item);
            }
            else {
              std::cout << "Send command '" << item->text << "' to iso TP" << std::endl << std::endl;
              std::lock_guard<std::mutex> guard(mtx);
              veccmd = &item->cmd;
              cmd_ready = true;
            }
          }
        }
        else if (scaned == 0) {
          // exit here
        }
        else {
          // print again
        }
      }
      else {
        // try to go up
        if (deep.empty() == false) {
          item = deep.top();
          deep.pop();
        }
        else {
          item = root;
        }
      }
    } // while
  }

  bool IsCmd() const {
    return cmd_ready;
  }

  std::vector<uint8_t> GetData() {
    std::lock_guard<std::mutex> guard(mtx);
    auto retv = *veccmd;
    cmd_ready = false;
    return retv;
  }

 private:
  Menu* const root{nullptr};
  std::stack<Menu*> deep;
  std::vector<uint8_t>* veccmd;
  std::mutex mtx;
  bool cmd_ready {false};
};
