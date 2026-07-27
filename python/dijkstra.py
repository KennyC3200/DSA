# Time complexity: O(E*log(V))
# This is a Single Source Shortest Path (SSSP) algorithm for graphs with non-negative edge weights
# This constraint on dijkstra's ensures that once a node has been visited its optimal distance
# cannot be improved. This property is important because it allows dijkstra's to act in a greedy
# manner by always selecting the next most promising node

# If input array is sorted, then
#     - Binary search

# If input array is not necessarily sorted, then
#     - Sliding window
#     - Two pointers

# If asked for all permutations/combinations/subsets, then
#     - Backtracking

# If given a tree/graph/grid, then
#     - DFS
#     - BFS

# If given a linked list then
#     - Two pointers

# If recursion is banned then
#     - Stack

# If must solve in-place then
#     - Swap corresponding values
#     - Store one or more different values in the same pointer

# If asked for maximum/minimum subarray/subset/options then
#     - Dynamic programming

# If asked for top/least K items then
#     - Heap

# If asked for common strings then
#     - Map
#     - Trie

# Else
#     - Map/Set for O(1) time & O(n) space
#     - Sort input for O(nlogn) time and O(1) space
    
# - A subarray or substring will always be contiguous, but a subsequence need not be contiguous, i.e., subsequences are not required to occupy consecutive positions within the original sequences.

dist = []