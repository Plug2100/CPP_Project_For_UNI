#include "graph_generator.hpp"
#include <algorithm>
#include <atomic>
#include <list>
#include <random>

namespace {

static constexpr float kGreenEdgeProbability = 0.1;
static constexpr float kRedEdgeProbability = 0.33;
static constexpr uni_course_cpp::Graph::Depth kDepthDifferenceRed = 2;
static constexpr uni_course_cpp::Graph::Depth kDepthDifferenceYellow = 1;
static constexpr uni_course_cpp::Graph::Depth kYellowDepthGap = 1;
static constexpr uni_course_cpp::Graph::Depth kYellowInitialDepth = 1;
static constexpr uni_course_cpp::Graph::Depth kRedInitialDepth = 1;
const int kMaxThreadsCount = std::thread::hardware_concurrency();

bool random_boolean(double probability) {
  std::random_device rd{};
  std::mt19937 rng{rd()};
  std::bernoulli_distribution d(probability);
  return d(rng);
}

int random_int(int limit) {
  std::random_device rd;
  std::mt19937 rng(rd());
  std::uniform_int_distribution<int> uni(0, limit);
  return uni(rng);
}

int get_random_vertex_id(
    const std::vector<uni_course_cpp::Graph::VertexId>& vertex_ids) {
  return vertex_ids.at(random_int(vertex_ids.size() - 1));
}

std::vector<uni_course_cpp::Graph::VertexId> get_unconnected_vertex_ids(
    std::mutex& edge_generator_mutex,
    const uni_course_cpp::Graph& graph,
    uni_course_cpp::Graph::VertexId vertex_id) {
  std::vector<uni_course_cpp::Graph::VertexId> suitable_vertices;
  for (const auto candidate_id :
       graph.vertex_ids_at_depth(graph.get_vertex_depth(vertex_id) + 1)) {
    const auto has_edge = [&graph, &edge_generator_mutex, vertex_id,
                           candidate_id]() {
      const std::lock_guard<std::mutex> lock(edge_generator_mutex);
      return graph.has_edge(vertex_id, candidate_id);
    }();
    if (!has_edge) {
      suitable_vertices.emplace_back(candidate_id);
    }
  }
  return suitable_vertices;
}
}  // namespace

namespace uni_course_cpp {

Graph GraphGenerator::generate() const {
  auto graph = Graph();
  if (params_.depth() == 0) {
    return graph;
  }
  generate_grey_edges(graph);
  std::mutex edge_generator_mutex;
  std::thread green_thread([this, &graph, &edge_generator_mutex]() {
    generate_green_edges(graph, edge_generator_mutex);
  });
  std::thread yellow_thread([this, &graph, &edge_generator_mutex]() {
    generate_yellow_edges(graph, edge_generator_mutex);
  });
  std::thread red_thread([this, &graph, &edge_generator_mutex]() {
    generate_red_edges(graph, edge_generator_mutex);
  });
  green_thread.join();
  yellow_thread.join();
  red_thread.join();
  return graph;
}

void GraphGenerator::generate_grey_branch(Graph& graph,
                                          std::mutex& edge_generator_mutex,
                                          Graph::VertexId vertex_id,
                                          Graph::Depth current_depth) const {
  const double probability = (params_.depth() - current_depth) /
                             ((double)params_.depth() - kGraphDefaultDepth);
  if (!random_boolean(probability))
    return;
  const auto new_vertex_id = [&graph, &edge_generator_mutex, vertex_id]() {
    const std::lock_guard lock(edge_generator_mutex);
    const auto new_vertex_id = graph.add_vertex();
    graph.add_edge(vertex_id, new_vertex_id);
    return new_vertex_id;
  }();

  for (int i = 0; i < params_.new_vertices_count(); i++) {
    generate_grey_branch(graph, edge_generator_mutex, new_vertex_id,
                         current_depth + 1);
  }
}

void GraphGenerator::generate_grey_edges(Graph& graph) const {
  const Graph::VertexId first_vertex_id = graph.add_vertex();
  if (params_.depth() == 1)
    return;
  using JobCallback = std::function<void()>;
  auto jobs = std::list<JobCallback>();
  std::mutex edge_generator_mutex;
  std::atomic<bool> should_terminate = false;
  std::atomic<int> jobs_counter = params_.new_vertices_count();
  for (int i = 0; i < params_.new_vertices_count(); i++) {
    jobs.push_back([&graph, &edge_generator_mutex, first_vertex_id,
                    &jobs_counter, this]() {
      generate_grey_branch(graph, edge_generator_mutex, first_vertex_id,
                           kGraphDefaultDepth);
      jobs_counter--;
    });
  }
  std::mutex job_mutex;

  const auto worker = [&should_terminate, &job_mutex, &jobs]() {
    while (true) {
      if (should_terminate) {
        return;
      }
      const auto job_optional = [&job_mutex,
                                 &jobs]() -> std::optional<JobCallback> {
        const std::lock_guard lock(job_mutex);
        if (!jobs.empty()) {
          auto job = jobs.front();
          jobs.pop_front();
          return job;
        }
        return std::nullopt;
      }();
      if (job_optional.has_value()) {
        const auto& job = job_optional.value();
        job();
      }
    }
  };

  const auto threads_count =
      std::min(kMaxThreadsCount, params_.new_vertices_count());
  auto threads = std::vector<std::thread>();
  threads.reserve(threads_count);

  for (int i = 0; i < threads_count; ++i) {
    threads.emplace_back(worker);
  }

  while (jobs_counter != 0) {
  }

  should_terminate = true;
  for (auto& thread : threads) {
    thread.join();
  }
}

void GraphGenerator::generate_green_edges(
    Graph& graph,
    std::mutex& edge_generator_mutex) const {
  const auto& vertices = graph.get_vertices();
  std::for_each(vertices.cbegin(), vertices.cend(),
                [&graph, &edge_generator_mutex](const auto& vertex) {
                  if (random_boolean(kGreenEdgeProbability)) {
                    const std::lock_guard lock(edge_generator_mutex);
                    graph.add_edge(vertex.id, vertex.id);
                  }
                });
}

void GraphGenerator::generate_yellow_edges(
    Graph& graph,
    std::mutex& edge_generator_mutex) const {
  if (params_.depth() < 3)
    return;
  const double probability_per_step =
      1.0 /
      ((double)graph.get_depth() - (kGraphDefaultDepth + kYellowDepthGap));
  for (int depth = kYellowInitialDepth;
       depth <= graph.get_depth() - kDepthDifferenceYellow; depth++) {
    const auto& vertices_at_current_depth = graph.vertex_ids_at_depth(depth);
    std::for_each(
        vertices_at_current_depth.cbegin(), vertices_at_current_depth.cend(),
        [&graph, &edge_generator_mutex, depth,
         probability_per_step](auto vertex_id) {
          if (random_boolean(depth * probability_per_step)) {
            const auto unconnected_vertex_ids = get_unconnected_vertex_ids(
                edge_generator_mutex, graph, vertex_id);
            if (!unconnected_vertex_ids.empty()) {
              const std::lock_guard lock(edge_generator_mutex);
              graph.add_edge(vertex_id,
                             get_random_vertex_id(unconnected_vertex_ids));
            }
          }
        });
  }
}

void GraphGenerator::generate_red_edges(
    Graph& graph,
    std::mutex& edge_generator_mutex) const {
  if (params_.depth() < 3)
    return;
  for (int depth = kRedInitialDepth;
       depth <= graph.get_depth() - kDepthDifferenceRed; depth++) {
    const auto& vertices_at_current_depth = graph.vertex_ids_at_depth(depth);
    std::for_each(
        vertices_at_current_depth.cbegin(), vertices_at_current_depth.cend(),
        [&graph, &edge_generator_mutex, depth](auto vertex_id) {
          if (random_boolean(kRedEdgeProbability)) {
            const auto& possible_vertices =
                graph.vertex_ids_at_depth(depth + kDepthDifferenceRed);

            const std::lock_guard lock(edge_generator_mutex);
            graph.add_edge(vertex_id, get_random_vertex_id(possible_vertices));
          }
        });
  }
}
}  // namespace uni_course_cpp
