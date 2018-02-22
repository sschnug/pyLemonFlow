#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <iostream>
#include <vector>
#include <lemon/list_graph.h>
#include <lemon/network_simplex.h>

using namespace lemon;
using namespace std;
namespace py = pybind11;

typedef ListDigraph GraphT;
typedef GraphT::ArcMap<int> CapMapT;
typedef GraphT::ArcMap<int> WeightMapT;
typedef GraphT::Node NodeT;
typedef GraphT::Arc ArcT;

class Graph
{
  GraphT g;
  std::map<NodeT, int> node2id;  // TODO: make unordered_map?
  std::map<int, NodeT> id2node;  // TODO: replace with vector

  CapMapT capacities;
  WeightMapT weights;

public:
  Graph() : capacities(g), weights(g) { }

  void reserve_node(int n);
  void reserve_arc(int n);
  int add_node(int id);
  int add_arc(int source_id, int target_id, int capacity, int weight);
  std::pair<int, std::vector<int> > min_cost_max_flow(int source_id, int sink_id);
};

void Graph::reserve_node(int n) // TODO: unclear what n means
{
  g.reserveNode(n);
}

void Graph::reserve_arc(int n) // TODO: unclear what n means
{
  g.reserveArc(n);
}

int Graph::add_node(int id)
{
  //std::cout << "a";
  NodeT node = g.addNode();
  node2id.insert(std::make_pair(node, id));
  id2node.insert(std::make_pair(id, node));
  //std::cout << "_";
  return 1;
}

int Graph::add_arc(int source_id, int target_id, int capacity, int weight)
{
  //std::cout << "b";
  NodeT source = id2node[source_id];
  NodeT target = id2node[target_id];
  ArcT arc = g.addArc(source, target);
  capacities[arc] = capacity;
  weights[arc] = weight;
  //std::cout << "_";
  return 1;
}

std::pair<int, std::vector<int> > Graph::min_cost_max_flow(int source_id, int sink_id)
{
  NetworkSimplex<ListDigraph> ns(g);
  NodeT source_node = id2node[source_id];
  NodeT sink_node = id2node[sink_id];
  ns.upperMap(capacities).costMap(weights).stSupply(source_node, sink_node, 1000000); // TODO WHY 1000???
  ns.run();

  std::vector<int> solution_path;
  NodeT current_node = source_node;
  solution_path.push_back(node2id[current_node]);
  for (ListDigraph::OutArcIt a(g, current_node); a != INVALID; ++a)
  {
    if (ns.flow(a) > 0.9)
    {
      current_node = g.target(a);
      solution_path.push_back(node2id[current_node]);
    }
  }

  return std::make_pair(ns.totalCost(), solution_path);
}

PYBIND11_PLUGIN(pyLemonFlow) {
    py::module m("pyLemonFlow", "pybind11 example plugin");

    py::class_<Graph>(m, "Graph")
      .def(py::init<>())
      .def("reserve_node", &Graph::reserve_node)
      .def("reserve_arc", &Graph::reserve_arc)
      .def("add_node", &Graph::add_node)
      .def("add_arc", &Graph::add_arc)
      .def("min_cost_max_flow", &Graph::min_cost_max_flow);

    return m.ptr();
}
