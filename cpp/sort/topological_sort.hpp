#include "../arena_allocator/stack_arena.hpp"
#include "../graph/node_graph.hpp"
#include <unordered_map>
#include <queue>
#include <utility>

template <typename Graph, typename Alloc = std::allocator<typename Graph::value_type>>
class TopoSort {
public:
    using value_type = typename Graph::value_type;
    using size_type = typename Graph::size_type;
    using allocator_type = Alloc;

private:
    template <typename T>
    using rebind_alloc = typename std::allocator_traits<allocator_type>::template rebind_alloc<T>;

    using Queue = std::queue<size_type, std::deque<size_type, rebind_alloc<size_type>>>;
    using Hashmap = std::unordered_map<
        value_type, size_type, std::hash<value_type>, std::equal_to<value_type>, 
        rebind_alloc<std::pair<const value_type, size_type>>>;

    template <typename T> using List = std::vector<T, rebind_alloc<T>>;

public:
    TopoSort(Graph& graph, Alloc alloc = std::allocator<value_type>()) 
        : m_graph(graph) 
        , m_alloc(alloc) {}
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

        // TODO: maybe optimize this to be a bitmap instead to save memory
        //       also, the std::fill is kind of unecessary tbh
        List<bool> visited(n, m_alloc);
        std::fill(visited.begin(), visited.end(), false);

        size_type list_idx = n - 1;
        for (size_type i = 0; i < n; i++) {
            if (!visited[i]) list_idx = DFS(i, visited, list, list_idx);
        }
        return list;
    }

    // Iterative DFS approach
    List<value_type> SortDFSIteratively() {
        size_type n = m_graph.Size();
        List<value_type> ordering(n, m_alloc);

        // TODO: maybe optimize this to be a bitmap instead to save memory
        //       also, the std::fill is kind of unecessary tbh
        List<bool> visited(n, m_alloc);
        std::fill(visited.begin(), visited.end(), false);

        size_type ordering_idx = n - 1;
        for (size_type starting_node = 0; starting_node < n; starting_node++) {
            if (visited[starting_node]) continue;

            List<size_type> stack(m_alloc);
            stack.reserve(n);
            stack.push_back(starting_node);
            while (!stack.empty()) {
                size_type node = stack.back();
                stack.pop_back();

                if (visited[node]) {
                    ordering[ordering_idx--] = m_graph[node].Val();
                } else {
                    stack.push_back(node);
                    visited[node] = true;

                    auto& edges = m_graph[node].Edges();
                    for (size_type& adj : edges) {
                        if (!visited[adj]) stack.push_back(adj);
                    }
                }
            }
        }

        return ordering;
    }

    static void PrintList(List<value_type> list, std::ostream& os = std::cout) {
        for (auto& node_val : list) os << node_val << ' ';
        os << std::endl;
    }

    static bool Sorted(
            Graph& graph, 
            List<value_type> list, 
            Alloc alloc = std::allocator<value_type>()) 
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
    Alloc m_alloc;
};
