#pragma once
#include <bits/stdc++.h>

class Logger {
 public:
  bool enable;
  void log(std::string str) {
    if (enable) std::cout << str << std::endl;
  }
};

Logger logger = Logger();
