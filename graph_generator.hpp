#pragma once

#include <mutex>
#include <thread>
#include "graph.hpp"

namespace uni_course_cpp {
class GraphGenerator {
 public:
  struct Params {
   public:
    explicit Params(Graph::Depth depth, int new_vertices_count)
        : depth_(depth), new_vertices_count_(new_vertices_count) {}

    Graph::Depth depth() const { return depth_; }
    int new_vertices_count() const { return new_vertices_count_; }

   private:
    Graph::Depth depth_ = 0;
    int new_vertices_count_ = 0;
  };

  explicit GraphGenerator(const Params&& params) : params_(params) {}

  Graph generate() const;

 private:
  Params params_;

  void generate_grey_edges(Graph& graph) const;

  void generate_green_edges(Graph& graph,
                            std::mutex& edge_generator_mutex) const;

  void generate_yellow_edges(Graph& graph,
                             std::mutex& edge_generator_mutex) const;

  void generate_red_edges(Graph& graph, std::mutex& edge_generator_mutex) const;

  void generate_grey_branch(Graph& graph,
                            std::mutex& edge_generator_mutex,
                            Graph::VertexId vertex_id,
                            Graph::Depth current_depth) const;
};
}  // namespace uni_course_cpp
