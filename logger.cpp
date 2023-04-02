#include "logger.hpp"
#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>
#include "configs.hpp"

namespace {

std::string get_current_date_time() {
  const auto date_time = std::chrono::system_clock::now();
  const auto date_time_t = std::chrono::system_clock::to_time_t(date_time);
  std::stringstream date_time_string;
  date_time_string << std::put_time(std::localtime(&date_time_t),
                                    "%Y.%m.%d %H:%M:%S");
  return date_time_string.str();
}
}  // namespace

namespace uni_course_cpp {

Logger& Logger::get_logger() {
  static Logger logger;
  return logger;
}

void Logger::log(const std::string& string) {
  const std::string log_string = get_current_date_time() + " " + string;

  const std::lock_guard lock(mutex_);

  std::cout << log_string << std::endl;
  log_file_ << log_string << std::endl;
}

Logger::Logger() : log_file_(config::kLogFilePath){};

}  // namespace uni_course_cpp
