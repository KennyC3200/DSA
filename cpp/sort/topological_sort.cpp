#include "../arena_allocator/stack_arena.hpp"
#include "../graph/node_graph.hpp"
#include <unordered_map>
#include <queue>
#include <utility>

// TODO: What if I want to choose a different allocator than the graph's allocator?
template <typename Graph>
class TopoSort {
private:
    using value_type = typename Graph::value_type;
    using size_type = typename Graph::size_type;
    using allocator_type = typename Graph::allocator_type;

    template <typename T>
    using Alloc = typename std::allocator_traits<allocator_type>::template rebind_alloc<T>;

    using Queue = std::queue<size_type, std::deque<size_type, Alloc<size_type>>>;
    using Hashmap = std::unordered_map<
        value_type, size_type, std::hash<value_type>, std::equal_to<value_type>, 
        Alloc<std::pair<const value_type, size_type>>>;

    template <typename T> using List = std::vector<T, Alloc<T>>;

public:
    TopoSort(Graph& graph) : m_graph(graph) , m_alloc(graph.GetAllocator()) {}
    ~TopoSort() = default;

    List<value_type> SortBFS() {
        size_type n = m_graph.Size();
        List<value_type> out(m_alloc);
        List<size_type> indegrees(n, m_alloc);
        Queue queue(m_alloc);

        // Compute indegrees
        for (size_type i = 0; i < n; i++) {
            auto& edges = m_graph[i].Edges();
            for (size_type& adj : edges) indegrees[adj] += 1;
        }

        // Add nodes with indegree == 0 to the queue
        for (size_type i = 0; i < n; i++) {
            if (indegrees[i] == 0) queue.push(i);
        }

        // Kahn's Algorithm
        while (!queue.empty()) {
            size_type node = queue.front();
            queue.pop();
            out.push_back(m_graph[node].Val());

            auto& edges = m_graph[node].Edges();
            for (size_type& adj : edges) {
                indegrees[adj] -= 1;
                if (indegrees[adj] == 0) queue.push(adj);
            }
        }

        return out;
    }

    // DFS approach
    List<value_type> SortDFS() {
        size_type n = m_graph.Size();
        List<value_type> list(n, m_alloc);
        List<bool> visited(n, m_alloc);
        std::fill(visited.begin(), visited.end(), false);
        size_type list_idx = n - 1;
        for (size_type i = 0; i < n; i++) {
            if (!visited[i]) list_idx = DFS(i, visited, list, list_idx);
        }
        return list;
    }

    static void PrintList(List<value_type> list, std::ostream& os = std::cout) {
        for (auto& node_val : list) os << node_val << ' ';
        os << std::endl;
    }

    static bool Sorted(
            Graph& graph, 
            List<value_type> list, 
            Alloc<allocator_type> alloc = std::allocator<value_type>()) 
    {
        size_type n = list.size();
        Hashmap node_positions(n, alloc);

        for (size_type i = 0; i < n; i++) node_positions[list[i]] = i;
        for (size_type i = 0; i < n; i++) {
            auto& edges = graph[i].Edges();
            for (size_type& adj : edges) {
                if (node_positions[graph[adj].Val()] < node_positions[graph[i].Val()]) return false;
            }
        }

        return true;
    }

private:
    // Recursive DFS
    size_type DFS(size_type node, List<bool>& visited, List<value_type>& list, size_type list_idx) {
        visited[node] = true;
        auto& edges = m_graph[node].Edges();
        for (size_type& adj : edges) {
            if (!visited[adj]) list_idx = DFS(adj, visited, list, list_idx);
        }

        list[list_idx] = m_graph[node].Val();
        return list_idx - 1;
    }

    Graph& m_graph;
    Alloc<allocator_type> m_alloc;
};

constexpr std::size_t ARENA_BYTES = 2048;
constexpr std::size_t NODES = 8;

int main(void) {
    // using NodeType = int;
    // using Alloc = short_alloc<NodeType, ARENA_BYTES>;
    // using Graph = NodeGraph<NodeType, Alloc>;
    // using Topo = TopoSort<Graph>;

    // Alloc::arena_type arena;
    // Graph graph(arena);
    using Graph = NodeGraph<int>;
    using Topo = TopoSort<Graph>;
    Graph graph;

    for (std::size_t i = 0; i < NODES; i++) graph.AddNode(i);

    graph.AddEdge(5, 0);
    graph.AddEdge(5, 2);
    graph.AddEdge(4, 0);
    graph.AddEdge(4, 1);
    graph.AddEdge(2, 3);
    graph.AddEdge(3, 1);

    Topo topo(graph);
    graph.Print();

    auto bfs = topo.SortBFS();
    Topo::PrintList(bfs);
    std::cout << (Topo::Sorted(graph, bfs) ? "True" : "False") << std::endl;

    auto dfs = topo.SortDFS();
    Topo::PrintList(dfs);
    std::cout << (Topo::Sorted(graph, dfs) ? "True" : "False") << std::endl;

    return 0;
}