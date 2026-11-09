#include <vector>

int main() {
    // If adding element: add to the very end and then bubble it up
    // If removing root element, move the last element to the root and bubble it down

    // For a node with index idx, we can find the parent, left, and right nodes:
    // parent = (idx - 1) / 2
    // left   = idx * 2 + 1
    // right  = idx * 2 + 2
}