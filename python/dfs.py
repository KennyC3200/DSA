# The time complexity is O(|V| + |E|): worst case we visit every node and traverse every edge
# Data structures: stack (or you can use recursion) and set (if not a tree)
# https://www.youtube.com/watch?v=Urx87-NMm6c&t=3s

from collections import deque

graph = {
    'A': ['B', 'G'],
    'B': ['C', 'D', 'E'],
    'C': [],
    'D': [],
    'E': ['F'],
    'F': [],
    'G': ['H'],
    'H': ['I'],
    'I': []
}

def dfs(graph, node, target):
    found = False

    # Also, we do not need the visited set for a tree
    visited = set()
    stack = deque()

    visited.add(node)
    stack.append(node)

    while stack:
        s = stack.pop()
        if s == target:
            found = True
        print(s, end=' ')

        # You need to use reversed(graph[s]) so that the leftmost node is at the top of the stack
        # E.g. reversed(graph['A']) = ['G', 'B'] so in the stack 'G' is at the bottom and 'B' is at the top
        # If we didn't reverse it, then the stack would cause it to search from the right node before the left
        for n in reversed(graph[s]):
            if n not in visited:
                visited.add(n)
                stack.append(n)

    if found:
        print('Found')
    else:
        print('Not Found')

dfs(graph, 'A', 'D')