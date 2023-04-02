#include "graph_printing.hpp"
#include <sstream>
#include <string>

namespace {
static constexpr int kDefaultDepth = 1;

std::string print_vertices(const uni_course_cpp::Graph& graph) {
  std::stringstream string_to_print;
  string_to_print << "{amount: " << graph.get_vertices().size()
                  << ", distribution: [";
  const uni_course_cpp::Graph::Depth graph_depth = graph.get_depth();
  for (int depth_now = kDefaultDepth; depth_now <= graph_depth; depth_now++) {
    if (depth_now != kDefaultDepth) {
      string_to_print << ", ";
    }
    string_to_print << graph.vertex_ids_at_depth(depth_now).size();
  }
  string_to_print << "]},";
  return string_to_print.str();
}

std::string print_edges(const uni_course_cpp::Graph& graph) {
  std::stringstream string_to_print;
  const auto& edges = graph.get_edges_ids_to_edges();
  string_to_print << "{amount: " << edges.size() << ", distribution: {";

  std::unordered_map<uni_course_cpp::Graph::Edge::Color, int>
      amount_of_edges_by_color;
  for (const auto& [edge_id, edge] : edges) {
    amount_of_edges_by_color[edge.color]++;
  }
  for (auto it = amount_of_edges_by_color.cbegin();
       it != amount_of_edges_by_color.cend(); it++) {
    if (it != amount_of_edges_by_color.cbegin()) {
      string_to_print << ", ";
    }
    string_to_print << uni_course_cpp::printing::print_edge_color(it->first)
                    << ": " << it->second;
  }
  string_to_print << "}}";
  return string_to_print.str();
}

}  // namespace

namespace uni_course_cpp {
namespace printing {

std::string print_edge_color(uni_course_cpp::Graph::Edge::Color color) {
  switch (color) {
    case uni_course_cpp::Graph::Edge::Color::Gray:
      return "gray";
    case uni_course_cpp::Graph::Edge::Color::Green:
      return "green";
    case uni_course_cpp::Graph::Edge::Color::Yellow:
      return "yellow";
    case uni_course_cpp::Graph::Edge::Color::Red:
      return "red";
  }
}

std::string print_graph(const Graph& graph) {
  std::stringstream string_to_print;
  string_to_print << "{\n"
                  << "\tdepth: " << graph.get_depth() << "\n";
  string_to_print << "\tvertices: " << print_vertices(graph) << "\n";
  string_to_print << "\tedges: " << print_edges(graph) << "\n"
                  << "{";
  return string_to_print.str();
}

}  // namespace printing
}  // namespace uni_course_cpp
