#include "dijkstra.h"
#include <queue>
#include <vector>
#include <climits>

Dijkstra::Dijkstra(int v) : V(v) {
    adj.resize(V);
}

void Dijkstra::addEdge(int u, int v, int w) {
    if (u >= V || v >= V || u < 0 || v < 0) return;
    adj[u].push_back({v, w});
    adj[v].push_back({u, w});
}

std::vector<int> Dijkstra::shortestPath(int src) {
    std::vector<int> dist(V, INT_MAX);
    std::priority_queue<std::pair<int, int>,
                        std::vector<std::pair<int, int>>,
                        std::greater<std::pair<int, int>>> pq;

    dist[src] = 0;
    pq.push({0, src});

    while (!pq.empty()) {
        int d = pq.top().first;
        int u = pq.top().second;
        pq.pop();

        for (auto& edge : adj[u]) {
            int v = edge.first;
            int w = edge.second;

            if (dist[u] + w < dist[v]) {
                dist[v] = dist[u] + w;
                pq.push({dist[v], v});
            }
        }
    }

    return dist;
}