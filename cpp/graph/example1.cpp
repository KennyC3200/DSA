#include "node_graph.hpp"

int main() {
    NodeGraph<int> graph;

    auto a = graph.AddNode(1);
    auto b = graph.AddNode(2);
    auto c = graph.AddNode(3);

    graph.AddEdge(a, b);
    graph.AddEdge(a, c);
    graph.AddEdge(b, c);
    graph.Print();
}
