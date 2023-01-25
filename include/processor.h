#ifndef PROCESSOR_H
#define PROCESSOR_H

#include <vector>
#include <string>

using std::vector;
using std::string;

class Processor {
 public:
  float Utilization();

 private:
    long last_idle_, last_total_ = 0;
};

#endif