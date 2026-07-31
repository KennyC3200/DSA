# The time complexity is O(|V| + |E|)
# https://youtu.be/7J3GadLzydI
# https://leetcode.com/discuss/post/1078072/introduction-to-topological-sort-by-sinc-i0ii/

from collections import deque
import string

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

def kahn_algorithm(graph) -> list:
    # BFS algorithm
    indegree = {x: 0 for x in string.ascii_uppercase[:len(graph)]}
    out = []
    queue = deque()

    # Compute indegrees
    for node in graph:
        for next in graph[node]:
            indegree[next] += 1

    # Append nodes with indegree 0 into the queue
    for node in indegree:
        if indegree[node] == 0:
            queue.append(node)

    while queue:
        node = queue.popleft()
        out.append(node)

        for next in graph[node]:
            indegree[next] -= 1
            if indegree[next] == 0:
                queue.append(next)

    return out

def topo_dfs(graph):
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

print(topo_dfs(graph))
print(kahn_algorithm(graph))