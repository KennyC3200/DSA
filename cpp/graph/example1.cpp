#include "node_graph.hpp"

int main() {
    NodeGraph<int> graph;

    // a, b, c are the indices in the graph
    auto a = graph.AddNode(1);
    auto b = graph.AddNode(2);
    auto c = graph.AddNode(3);

    // Connect the nodes
    graph.AddEdge(a, b);
    graph.AddEdge(a, c);
    graph.AddEdge(b, c);

    // Print the graph
    for (int i = 0, sz = graph.Size(); i < sz; i++) {
        std::cout << graph[i].Val() << " -> ";
        for (auto &adj : graph[i].Edges()) {
            std::cout << graph[adj].Val() << ' ';
        }
        std::cout << '\n';
    }
}