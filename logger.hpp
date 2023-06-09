#pragma once

#include <fstream>
#include <mutex>
#include <string>

namespace uni_course_cpp {

class Logger {
 public:
  static Logger& get_logger();
  void log(const std::string& string);

 private:
  Logger();
  Logger(const Logger&) = delete;
  Logger& operator=(const Logger&) = delete;
  Logger(Logger&&) = delete;
  Logger& operator=(Logger&&) = delete;
  std::mutex mutex_;
  std::ofstream log_file_;
};
}  // namespace uni_course_cpp
