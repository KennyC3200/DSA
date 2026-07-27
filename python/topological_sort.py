# The time complexity is O(|V| + |E|)
# https://youtu.be/7J3GadLzydI
# https://leetcode.com/discuss/post/1078072/introduction-to-topological-sort-by-sinc-i0ii/

graph = {
    'A': {'D'},
    'B': {'D'},
    'C': {'B'},
    'D': {'G', 'H'},
    'E': {'A', 'D', 'F'},
    'F': {'K', 'J'},
    'G': {'I'},
    'H': {'J', 'I'},
    'I': {'L'},
    'J': {'M', 'L'},
    'K': {'J'},
    'L': {},
    'M': {},
}

def topo_sort(graph):
    n = len(graph)
    visited = {node: False for node in graph}
    ordering = [''] * n
    ordering_idx = n - 1

    for node in graph:
        if visited[node] == False:
            ordering_idx = dfs(node, visited, ordering, ordering_idx, graph)
    return ordering

def dfs(node, visited, ordering: list, ordering_idx, graph):
    visited[node] = True

    for next in graph[node]:
        if visited[next] == False:
            ordering_idx = dfs(next, visited, ordering, ordering_idx, graph)

    ordering[ordering_idx] = node
    return ordering_idx - 1

print(topo_sort(graph))