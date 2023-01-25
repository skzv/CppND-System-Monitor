#include <vector>
#include <string>
#include <iostream>

#include "processor.h"
#include "linux_parser.h"

float Processor::Utilization() { 
    long idle = LinuxParser::IdleJiffies();
    long active = LinuxParser::ActiveJiffies();
    long total = idle + active;

    long total_delta = total - last_total_;
    long idle_delta = idle - last_idle_;
    float cpu_utilization = float(total_delta - idle_delta)/total_delta;

    last_idle_ = idle;
    last_total_ = total;

    return cpu_utilization; 
}