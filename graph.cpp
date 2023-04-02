#include "graph.hpp"
#include <cassert>
#include <stdexcept>

namespace uni_course_cpp {

Graph::Edge::Color Graph::calculate_edge_color(VertexId from_vertex_id,
                                               VertexId to_vertex_id) const {
  const auto from_vertex_depth = get_vertex_depth(from_vertex_id);

  const auto to_vertex_depth = get_vertex_depth(to_vertex_id);

  if (from_vertex_id == to_vertex_id) {
    return Edge::Color::Green;
  }
  if (edge_ids_connected_to_vertex(to_vertex_id).size() == 0) {
    return Edge::Color::Gray;
  }
  if (to_vertex_depth - from_vertex_depth == 1 &&
      !has_edge(from_vertex_id, to_vertex_id)) {
    return Edge::Color::Yellow;
  }
  if (to_vertex_depth - from_vertex_depth == 2) {
    return Edge::Color::Red;
  }
  throw std::runtime_error("Failed to determine color");
}

bool Graph::has_vertex(Graph::VertexId vertex_id) const {
  assert(vertex_id >= 0 && "vertex_id < 0");
  for (const auto& vertex : vertices_) {
    if (vertex.id == vertex_id) {
      return true;
    }
  }
  return false;
}

bool Graph::has_edge(VertexId first_vertex_id,
                     VertexId second_vertex_id) const {
  assert(first_vertex_id >= 0 && "first_vertex_id < 0");
  assert(second_vertex_id >= 0 && "second_vertex_id < 0");
  if (first_vertex_id != second_vertex_id) {
    for (const auto first_edge_id : adjacency_list_.at(first_vertex_id)) {
      for (const auto second_edge_id : adjacency_list_.at(second_vertex_id)) {
        if (first_edge_id == second_edge_id) {
          return true;
        }
      }
    }
  } else {
    for (const auto edge_id : adjacency_list_.at(first_vertex_id)) {
      if (edge_id_to_edge_.at(edge_id).color == Edge::Color::Green) {
        return true;
      }
    }
  }
  return false;
}

Graph::VertexId Graph::add_vertex() {
  const auto& new_vertex = vertices_.emplace_back(get_new_vertex_id());
  adjacency_list_[new_vertex.id] = {};
  vertex_depths_.emplace(new_vertex.id, kGraphDefaultDepth);
  depth_list_[kGraphDefaultDepth].emplace_back(new_vertex.id);

  return new_vertex.id;
}

void Graph::update_depth(VertexId first_vertex_id, VertexId second_vertex_id) {
  const Depth second_vertex_depth = get_vertex_depth(first_vertex_id) + 1;
  vertex_depths_[second_vertex_id] = second_vertex_depth;
  auto& vertices_at_default_depth = depth_list_.at(kGraphDefaultDepth);
  depth_list_[second_vertex_depth].emplace_back(second_vertex_id);
  for (auto vertex_id = vertices_at_default_depth.begin();
       vertex_id != vertices_at_default_depth.end(); ++vertex_id) {
    if (*vertex_id == second_vertex_id) {
      vertices_at_default_depth.erase(vertex_id);
      break;
    }
  }
}

void Graph::add_edge(VertexId first_vertex_id, VertexId second_vertex_id) {
  assert(has_vertex(first_vertex_id) && "first_vertex_id doesn't exist");
  assert(has_vertex(second_vertex_id) && "second_vertex_id doesn't exist");
  const auto color = calculate_edge_color(first_vertex_id, second_vertex_id);
  const auto new_edge_id = get_new_edge_id();
  edge_id_to_edge_.emplace(
      new_edge_id, Edge(new_edge_id, first_vertex_id, second_vertex_id, color));
  adjacency_list_[first_vertex_id].emplace_back(new_edge_id);
  if (color != Edge::Color::Green) {
    adjacency_list_[second_vertex_id].emplace_back(new_edge_id);
  }
  if (color == Edge::Color::Gray) {
    update_depth(first_vertex_id, second_vertex_id);
  }
}

}  // namespace uni_course_cpp
