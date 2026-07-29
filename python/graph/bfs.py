# Breadth = broad/wide. It will progress horizontally before vertically. That is, it explores all nodes in a layer before moving to the next depth level
# The time complexity is O(|V| + |E|): worst case we visit every node and traverse every edge
# Data structures: queue (FIFO)
# https://www.youtube.com/watch?v=HZ5YTanv5QE

from collections import deque

graph = {
    'A': ['B', 'C'],
    'B': ['D', 'E', 'F'],
    'C': ['G'],
    'D': [],
    'E': [],
    'F': ['H'],
    'G': ['I'],
    'H': [],
    'I': []
}

def bfs(graph, node, target):
    found = False

    visited = set()
    queue = deque()

    visited.add(node)
    queue.append(node)

    while queue:
        s = queue.popleft()
        if s == target:
            found = True
        print(s, end=' ')

        for n in graph[s]:
            if n not in visited:
                visited.add(n)
                queue.append(n)

    if found:
        print('Found')
    else:
        print('Not Found')

bfs(graph, 'A', 'D')