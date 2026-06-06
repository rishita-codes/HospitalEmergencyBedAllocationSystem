#ifndef DIJKSTRA_H
#define DIJKSTRA_H

#include <vector>

class Dijkstra {
private:
    int V;
    std::vector<std::vector<std::pair<int, int>>> adj;

public:
    Dijkstra(int v);
    void addEdge(int u, int v, int w);
    std::vector<int> shortestPath(int src);
};

#endif