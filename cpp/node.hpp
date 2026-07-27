#pragma once

template <typename T=int>
class Node {
public:
    Node(T val) {
        this->val = val;
    }

    T val;
    Node** adj;
};