#pragma once

#include <assert.h>
#include <stdlib.h>
#include <stdint.h>
#include <iostream>
#include <vector>
#include <stack>

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
  CliMen() : root(new Menu("hellllo")) {
    assert(root != nullptr);
    static Menu* top1_2 = new Menu("world");
    static Menu* top2_1 = new Menu("and");
    static Menu* top2_2 = new Menu("what");
    root->SetNext(top1_2);
    top1_2->SetDown(top2_1);
    top2_1->SetNext(top2_2);
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

      do {
        std::cout << item->text << std::endl;
        item = item->GetNext();
      }
      while (item != nullptr);

      std::string in;

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
              std::cout << "command level in " << item->text << std::endl;
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

 private:
  Menu* const root{nullptr};
  std::stack<Menu*> deep;
};
