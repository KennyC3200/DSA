# Tarjan's Strongly Connected Components
# Time complexity: O(V + E)
# https://youtu.be/wUgWX0nc4NY

from collections import deque

graph = {
    0: [1, 4],
    1: [5],
    2: [1, 3, 6],
    3: [6],
    4: [0, 5],
    5: [2, 6],
    6: [7],
    7: [3]
}

UNVISITED = -1
n = len(graph)

id = 0
stack = deque()
on_stack = [False] * n
ids = [0] * n
low = [0] * n
scc_cnt = 0

def tarjan_scc():
    for i in range(n):
        ids[i] = UNVISITED
    for i in range(n):
        if ids[i] == UNVISITED:
            dfs(i)
    return low

def dfs(at):
    global id, scc_cnt

    stack.append(at)
    on_stack[at] = True
    id += 1
    ids[at] = low[at] = id

    # Visit all neighbours and min low-link on callback
    for to in graph[at]:
        if ids[to] == UNVISITED:
            dfs(to)
        
        # Note that the start of the SCC will already be on the stack
        if on_stack[to]:
            low[at] = min(low[to], low[at])

    # After visiting all the neighbours of 'at'
    # if we're at the start of an SCC, empty the visited stack
    # until we're back at the start of the SCC
    if ids[at] == low[at]:
        while True:
            node = stack.pop()
            on_stack[node] = False
            low[node] = ids[at] # Or could set it equal to low[at]--they're the same
            if node == at:
                break
        scc_cnt += 1

print(tarjan_scc())
print(scc_cnt)