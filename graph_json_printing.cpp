#include "graph_json_printing.hpp"
#include <sstream>
#include <string>
#include "graph_printing.hpp"

namespace uni_course_cpp {
namespace printing {
namespace json {

std::string print_vertex(const Graph::Vertex& vertex, const Graph& graph) {
  const auto& connected_edge_ids =
      graph.edge_ids_connected_to_vertex(vertex.id);
  std::stringstream string_to_print;
  string_to_print << "{\"id\":" << vertex.id << ",\"edge_ids\":[";
  for (const auto id : connected_edge_ids) {
    string_to_print << id;
    string_to_print << ",";
  }
  if (connected_edge_ids.size() != 0) {
    string_to_print.seekp(-1, string_to_print.cur);
  }
  string_to_print << "], ";
  string_to_print << "\"depth\": " << graph.get_vertex_depth(vertex.id);
  string_to_print << '}';
  return string_to_print.str();
}

std::string print_edge(const Graph::Edge& edge) {
  std::stringstream string_to_print;
  string_to_print << "{\"id\": " << edge.id << ",\"vertex_ids\": ["
                  << edge.from_vertex_id << "," << edge.to_vertex_id << "], "
                  << "\"color\": \"" << print_edge_color(edge.color) << "\""
                  << "}";
  return string_to_print.str();
}

std::string print_graph(const Graph& graph) {
  std::stringstream string_to_print;
  string_to_print << "{"
                  << "\"depth\": " << graph.get_depth() << ", \"vertices\": [";
  for (const auto& vertex : graph.get_vertices()) {
    string_to_print << json::print_vertex(vertex, graph);
    string_to_print << ",";
  }
  if (graph.get_vertices().size() != 0) {
    string_to_print.seekp(-1, string_to_print.cur);
  }
  string_to_print << ']';
  string_to_print << ","
                  << "\t\"edges\": [";
  for (const auto& [edge_id, edge] : graph.get_edges_ids_to_edges()) {
    string_to_print << json::print_edge(edge);
    string_to_print << ",";
  }
  if (graph.get_edges_ids_to_edges().size() != 0) {
    string_to_print.seekp(-1, string_to_print.cur);
  }
  string_to_print << ']';
  string_to_print << "}"
                  << "\n";
  return string_to_print.str();
}
}  // namespace json
}  // namespace printing
}  // namespace uni_course_cpp
