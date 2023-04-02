#include <filesystem>
#include <fstream>
#include <iostream>
#include <mutex>
#include <string>
#include "configs.hpp"
#include "graph_generation_controller.hpp"
#include "graph_generator.hpp"
#include "graph_json_printing.hpp"
#include "graph_printing.hpp"
#include "logger.hpp"

static constexpr int kVerticesCount = 14;
static constexpr int kInvalidDepth = -1;
static constexpr int kInvalidNewVerticesCount = -1;
static constexpr int kInvalidNewGraphsCount = -1;
static constexpr int kInvalidThreadsCount = -1;

void write_to_file(const std::string& string_to_write,
                   const std::string& filename) {
  std::ofstream output_file(filename);
  output_file << string_to_write;
  output_file.close();
}

int handle_threads_count_input() {
  int threads = kInvalidThreadsCount;
  std::cout << "Plz write amount of threads ";
  while (threads == kInvalidThreadsCount) {
    int buffer;
    std::cin >> buffer;
    if (std::cin.fail()) {
      std::cin.clear();
      std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
      std::cout << "You didn't enter a number! Enter a number >= 0 ";
    } else if (buffer < 0)
      std::cout << "Print normal threads plz (>= 0) ";
    else {
      threads = buffer;
    }
  }
  return threads;
}

int handle_depth_input() {
  int depth = kInvalidDepth;
  std::cout << "Plz write depth ";
  while (depth == kInvalidDepth) {
    int buffer;
    std::cin >> buffer;
    if (std::cin.fail()) {
      std::cin.clear();
      std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
      std::cout << "You didn't enter a number! Enter a number >= 0 ";
    } else if (buffer < 0)
      std::cout << "Print normal depth plz (>= 0) ";
    else {
      depth = buffer;
    }
  }
  return depth;
}

int handle_new_vertices_count_input() {
  int new_vertices_count = kInvalidNewVerticesCount;
  std::cout << "Pls write amount of vertices ";
  while (new_vertices_count == kInvalidNewVerticesCount) {
    int buffer;
    std::cin >> buffer;
    if (std::cin.fail()) {
      std::cin.clear();
      std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
      std::cout << "You didn't enter a number! Enter a number >= 0 ";
    } else if (buffer < 0)
      std::cout << "Print normal amount of vertices plz (>= 0) ";
    else {
      new_vertices_count = buffer;
    }
  }
  return new_vertices_count;
}

int handle_graphs_count_input() {
  int new_graphs_count = kInvalidNewGraphsCount;
  std::cout << "Pls write graphs count ";
  while (new_graphs_count == kInvalidNewGraphsCount) {
    int buffer;
    std::cin >> buffer;
    if (std::cin.fail()) {
      std::cin.clear();
      std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
      std::cout << "You didn't enter a number! Enter a number >= 0 ";
    } else if (buffer < 0)
      std::cout << "Print normal amount of graphs plz (>= 0) ";
    else {
      new_graphs_count = buffer;
    }
  }
  return new_graphs_count;
}

void prepare_temp_directory() {
  std::filesystem::create_directory(uni_course_cpp::config::kTempDirectoryPath);
}

std::string generation_started_string(int number_of_graph) {
  return "Graph " + std::to_string(number_of_graph) + ", Generation Started";
}

std::string generation_finished_string(int number_of_graph,
                                       const std::string& graph_description) {
  return "Graph " + std::to_string(number_of_graph) + ", Generation Finished " +
         graph_description;
}

std::vector<uni_course_cpp::Graph> generate_graphs(
    uni_course_cpp::GraphGenerator::Params&& params,
    int graphs_count,
    int threads_count) {
  auto generation_controller = uni_course_cpp::GraphGenerationController(
      threads_count, graphs_count, std::move(params));

  auto& logger = uni_course_cpp::Logger::get_logger();

  auto graphs = std::vector<uni_course_cpp::Graph>();
  graphs.reserve(graphs_count);

  generation_controller.generate(
      [&logger](int index) { logger.log(generation_started_string(index)); },
      [&logger, &graphs](int index, uni_course_cpp::Graph&& graph) {
        const auto graph_description =
            uni_course_cpp::printing::print_graph(graph);
        logger.log(generation_finished_string(index, graph_description));
        const auto graph_json =
            uni_course_cpp::printing::json::print_graph(graph);
        write_to_file(graph_json, uni_course_cpp::config::kTempDirectoryPath +
                                      "graph_" + std::to_string(index) +
                                      ".json");
        graphs.push_back(std::move(graph));
      });

  return graphs;
}

int main() {
  const int depth = handle_depth_input();
  const int new_vertices_count = handle_new_vertices_count_input();
  const int graphs_count = handle_graphs_count_input();
  const int threads_count = handle_threads_count_input();
  prepare_temp_directory();

  auto params =
      uni_course_cpp::GraphGenerator::Params(depth, new_vertices_count);
  const auto graphs =
      generate_graphs(std::move(params), graphs_count, threads_count);
  return 0;
}
