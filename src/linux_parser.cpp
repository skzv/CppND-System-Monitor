#include <dirent.h>
#include <unistd.h>
#include <sstream>
#include <string>
#include <vector>
#include <iostream>
#include <unistd.h>

#include "linux_parser.h"

using std::stof;
using std::string;
using std::to_string;
using std::vector;

string LinuxParser::OperatingSystem() {
  string line;
  string key;
  string value;
  std::ifstream filestream(kOSPath);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ' ', '_');
      std::replace(line.begin(), line.end(), '=', ' ');
      std::replace(line.begin(), line.end(), '"', ' ');
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "PRETTY_NAME") {
          std::replace(value.begin(), value.end(), '_', ' ');
          return value;
        }
      }
    }
  }
  return value;
}

string LinuxParser::Kernel() {
  string os, kernel, version;
  string line;
  std::ifstream stream(kProcDirectory + kVersionFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> os >> version >> kernel;
  }
  return kernel;
}

vector<int> LinuxParser::Pids() {
  vector<int> pids;
  DIR* directory = opendir(kProcDirectory.c_str());
  struct dirent* file;
  while ((file = readdir(directory)) != nullptr) {
    // Is this a directory?
    if (file->d_type == DT_DIR) {
      // Is every character of the name a digit?
      string filename(file->d_name);
      if (std::all_of(filename.begin(), filename.end(), isdigit)) {
        int pid = stoi(filename);
        pids.push_back(pid);
      }
    }
  }
  closedir(directory);
  return pids;
}

// Read and return the system memory utilization
float LinuxParser::MemoryUtilization() { 
  string line;
  string key;
  string value;

  long mem_free, mem_total = 0;

  std::ifstream filestream(kProcDirectory + kMeminfoFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "MemTotal:") {
          mem_total = stol(value);
        } else if (key == "MemFree:") {
          mem_free = stol(value);
        }
      }
    }
  }

  if (mem_total == 0) {
    return 0;
  } 

  return float(mem_total - mem_free) / mem_total;
}

// Read and return the system uptime
long LinuxParser::UpTime() { 
  string uptime, idletime;
  string line;
  std::ifstream stream(kProcDirectory + kUptimeFilename);

  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> uptime >> idletime;
  }

  return stol(uptime); 
}

// Read and return the number of jiffies for the system
long LinuxParser::Jiffies() { 
  return IdleJiffies() + ActiveJiffies(); 
}

// Read and return the number of active jiffies for a PID
long LinuxParser::ActiveJiffies(int pid) { 
    std::ifstream stream(kProcDirectory + std::to_string(pid) + "/" + kStatFilename);

  if (stream.is_open()) {
    string line, token;
    std::getline(stream, line);
    std::istringstream linestream(line);
    long utime, stime, cutime, cstime;
    for (int i = 1; linestream >> token; i++) {
      switch (i) {
        case 14:
          utime = stol(token);
          break;
        case 15:
          stime = stol(token);
          break;
        case 16:
          cutime = stol(token);
          break;
        case 17:
          cstime = stol(token);
          break;
      }
    }
    long total_time = utime + stime + cutime + cstime;
    return total_time;
  }
  return 0;
}

// Read and return the number of active jiffies for the system
long LinuxParser::ActiveJiffies() {
  vector<string> utilization = CpuUtilization();
  return stol(utilization[kUser_]) 
                        + stol(utilization[kNice_]) 
                        + stol(utilization[kSystem_]) 
                        + stol(utilization[kIRQ_]) 
                        + stol(utilization[kSoftIRQ_]) 
                        + stol(utilization[kSteal_]); 
}

// Read and return the number of idle jiffies for the system
long LinuxParser::IdleJiffies() { 
  vector<string> utilization = CpuUtilization();
  return stol(utilization[kIdle_]) + stol(utilization[kIOwait_]); 
}

// Read and return CPU utilization
vector<string> LinuxParser::CpuUtilization() { 
  vector<string> utilization;
  string line, token;

  std::ifstream filestream(kProcDirectory + kStatFilename);
  if (filestream.is_open()) {
    std::getline(filestream, line); 
      std::istringstream linestream(line);
      linestream >> token; // eat first "cpu" token
      while (linestream >> token) {
        utilization.emplace_back(token);
      }
  }

  return utilization; 
}

// Read and return the total number of processes
int LinuxParser::TotalProcesses() { 
  string line;
  string key;
  string value;
  std::ifstream filestream(kProcDirectory + kStatFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "processes") {
          return stol(value);
        }
      }
    }
  }
  return 0;
}

// Read and return the number of running processes
int LinuxParser::RunningProcesses() { 
  string line;
  string key;
  string value;
  std::ifstream filestream(kProcDirectory + kStatFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "procs_running") {
          return stol(value);
        }
      }
    }
  }
  return 0;
}

// Read and return the command associated with a process
string LinuxParser::Command(int pid) { 
  string cmd;
  std::ifstream stream(kProcDirectory + std::to_string(pid) + "/" + kCmdlineFilename);

  if (stream.is_open()) {
    std::getline(stream, cmd);
  }

  return cmd; 
}

// Read and return the memory used by a process
string LinuxParser::Ram(int pid) { 
  string line;
  string key;
  string value;
  std::ifstream filestream(kProcDirectory + std::to_string(pid) + "/" + kStatusFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "VmSize:") {
          long ram_kb = stol(value);
          long ram_mb = ram_kb / 1000;
          return to_string(ram_mb);
        }
      }
    }
  }
  return "";
 }

// Read and return the user ID associated with a process
string LinuxParser::Uid(int pid) { 
  string line;
  string key;
  string value;
  std::ifstream filestream(kProcDirectory + std::to_string(pid) + "/" + kStatusFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "Uid:") {
          return value;
        }
      }
    }
  }
  return "";
}

// Read and return the user associated with a process
string LinuxParser::User(int pid) { 
  string uid = Uid(pid);

  string line;
  string username, x, token_uid;

  std::ifstream filestream(kPasswordPath);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ':', ' ');
      std::istringstream linestream(line);
      while (linestream >> username >> x >> token_uid) {
        if (token_uid == uid) {
          return username;
        }
      }
    }
  }
  return "";
}

// Read and return the uptime of a process
long LinuxParser::UpTime(int pid) { 
  std::ifstream stream(kProcDirectory + std::to_string(pid) + "/" + kStatFilename);

  if (stream.is_open()) {
    string line, token;
    std::getline(stream, line);
    std::istringstream linestream(line);
    for (int i = 0; i < 22 && linestream >> token; i++);
    long starttime = stol(token) / sysconf(_SC_CLK_TCK);
    long uptime = UpTime() - starttime;
    return uptime;
  }
  return 0; 
}
