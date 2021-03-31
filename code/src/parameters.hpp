#pragma once

#include <bits/stdc++.h>

namespace chr = std::chrono;

// This is to keep track of the runtime parameters.
// Currently only execution time is measured
class Parameters {
 private:
  chr::_V2::system_clock::time_point start_time;
  chr::_V2::system_clock::time_point end_time;
  chr::duration<double> elapsed_seconds;

 public:
  void initialize();
  double get_time_elapsed();
  void complete();
};

void Parameters::initialize() { start_time = std::chrono::system_clock::now(); }
void Parameters::complete() {
  end_time = std::chrono::system_clock::now();
  elapsed_seconds = end_time - start_time;
}
double Parameters::get_time_elapsed() { return elapsed_seconds.count(); }
