#pragma once
#include <unordered_map>
#include <vector>

namespace uni_course_cpp {

static constexpr int kGraphDefaultDepth = 1;

class Graph {
 public:
  using VertexId = int;
  using EdgeId = int;
  using Depth = int;

  struct Vertex {
    explicit Vertex(VertexId init_id) : id(init_id) {}
    const VertexId id = 0;
  };

  struct Edge {
    enum class Color { Gray, Green, Yellow, Red };
    Edge(EdgeId init_id,
         VertexId init_from_vertex_id,
         VertexId init_to_vertex_id,
         Color color)
        : id(init_id),
          from_vertex_id(init_from_vertex_id),
          to_vertex_id(init_to_vertex_id),
          color(color) {}

    const EdgeId id = 0;
    const VertexId from_vertex_id = 0;
    const VertexId to_vertex_id = 0;
    const Color color;
  };

  bool has_vertex(VertexId vertex_id) const;

  bool has_edge(VertexId first_vertex_id, VertexId second_vertex_id) const;

  VertexId add_vertex();

  void add_edge(VertexId first_vertex_id, VertexId second_vertex_id);

  void update_depth(VertexId first_vertex_id, VertexId second_vertex_id);

  const std::vector<EdgeId>& edge_ids_connected_to_vertex(
      VertexId vertex_id) const {
    return adjacency_list_.at(vertex_id);
  }

  const std::vector<Vertex>& get_vertices() const { return vertices_; }

  const std::unordered_map<EdgeId, Edge>& get_edges_ids_to_edges() const {
    return edge_id_to_edge_;
  }

  Depth get_vertex_depth(VertexId vertex_id) const {
    return vertex_depths_.at(vertex_id);
  }

  const std::vector<VertexId>& vertex_ids_at_depth(Depth depth) const {
    return depth_list_.at(depth);
  }

  int get_depth() const { return depth_list_.size(); }

 private:
  std::vector<Vertex> vertices_;
  std::unordered_map<VertexId, std::vector<EdgeId>> adjacency_list_;
  std::unordered_map<Depth, std::vector<VertexId>> depth_list_;
  std::unordered_map<VertexId, Depth> vertex_depths_;
  std::unordered_map<EdgeId, Edge> edge_id_to_edge_;

  VertexId vertex_id_counter_ = 0;
  EdgeId edge_id_counter_ = 0;
  VertexId get_new_vertex_id() { return vertex_id_counter_++; }
  EdgeId get_new_edge_id() { return edge_id_counter_++; }

  Edge::Color calculate_edge_color(VertexId from_vertex_id,
                                   VertexId to_vertex_id) const;
};
}  // namespace uni_course_cpp
