#include "graph_generation_controller.hpp"
#include <atomic>
#include <cassert>
#include <iostream>

namespace uni_course_cpp {

GraphGenerationController::GraphGenerationController(
    int threads_count,
    int graphs_count,
    GraphGenerator::Params&& graph_generator_params)
    : threads_count_(threads_count),
      graphs_count_(graphs_count),
      graph_generator_(std::move(graph_generator_params)) {
  Worker::GetJobCallback get_job_callback =
      [&jobs_ = jobs_,
       &job_mutex_ = job_mutex_]() -> std::optional<JobCallback> {
    const std::lock_guard lock(job_mutex_);
    if (!jobs_.empty()) {
      auto job = jobs_.front();
      jobs_.pop_front();
      return job;
    }
    return std::nullopt;
  };

  for (int i = 0; i < std::min(kMaxThreadsCount, threads_count_); i++) {
    workers_.emplace_back(get_job_callback);
  }
}

void GraphGenerationController::generate(
    const GenStartedCallback& gen_started_callback,
    const GenFinishedCallback& gen_finished_callback) {
  std::atomic<int> jobs_counter = graphs_count_;
  std::mutex callback_mutex;
  for (int i = 0; i < graphs_count_; i++) {
    jobs_.emplace_back([&gen_started_callback, &gen_finished_callback,
                        &jobs_counter, i, &graph_generator_ = graph_generator_,
                        &callback_mutex]() {
      {
        const std::lock_guard lock(callback_mutex);
        gen_started_callback(i);
      }
      auto graph = graph_generator_.generate();
      {
        const std::lock_guard lock(callback_mutex);
        gen_finished_callback(i, std::move(graph));
      }
      jobs_counter--;
    });
  }
  for (auto& worker : workers_) {
    worker.start();
  }
  while (jobs_counter != 0) {
  }

  for (auto& worker : workers_) {
    worker.stop();
  }
}

void GraphGenerationController::Worker::start() {
  assert(state_ != State::Working && "Worker is already working");
  state_ = State::Working;

  thread_ =
      std::thread([&state_ = state_, &get_job_callback_ = get_job_callback_]() {
        while (true) {
          if (state_ == State::ShouldTerminate) {
            return;
          }
          const auto job_optional = get_job_callback_();
          if (job_optional.has_value()) {
            const auto& job = job_optional.value();
            job();
          }
        }
      });
}

void GraphGenerationController::Worker::stop() {
  assert(state_ == State::Working && "Worker has been already stopped");

  state_ = State::ShouldTerminate;
  thread_.join();
}

GraphGenerationController::Worker::~Worker() {
  if (state_ == State::Working) {
    stop();
  }
}
}  // namespace uni_course_cpp
