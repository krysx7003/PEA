#pragma once
#include <vector>

struct Node {
    std::vector<int> path;
    int cost;
    int bound;
    int level=0;
    bool operator<(const Node& other) const {
        return bound > other.bound; 
    }
};