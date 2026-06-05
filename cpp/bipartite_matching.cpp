#include "bipartite_matching.h"
#include <vector>
#include <queue>
#include <algorithm>
#include <climits>

static bool helper(std::vector<std::vector<int>>& cost,
                   std::vector<int>& set2,
                   std::vector<int>& visited,
                   std::vector<int>& match,
                   int cur) {
    if (visited[cur]) return false;
    visited[cur] = 1;

    for (int j = 0; j < cost[cur].size(); j++) {
        if (cost[cur][j] == 0) {
            if (set2[j] == -1 || helper(cost, set2, visited, match, set2[j])) {
                match[cur] = j;
                set2[j] = cur;
                return true;
            }
        }
    }
    return false;
}

static void mark(std::vector<std::vector<int>>& cost,
                 std::vector<int>& match,
                 std::vector<int>& set2,
                 std::vector<int>& row_covered,
                 std::vector<int>& col_covered) {
    int n1 = cost.size();
    int n2 = cost[0].size();
    std::queue<int> q;

    for (int i = 0; i < n1; i++) {
        if (match[i] == -1) {
            q.push(i);
            row_covered[i] = 1;
        }
    }

    while (!q.empty()) {
        int r = q.front();
        q.pop();
        for (int c = 0; c < n2; c++) {
            if (cost[r][c] == 0 && !col_covered[c]) {
                col_covered[c] = 1;
                if (set2[c] != -1 && !row_covered[set2[c]]) {
                    row_covered[set2[c]] = 1;
                    q.push(set2[c]);
                }
            }
        }
    }
}

std::vector<int> hungarian(const std::vector<std::vector<int>>& cost_orig) {
    int n1 = cost_orig.size();
    int n2 = cost_orig[0].size();
    std::vector<std::vector<int>> cost = cost_orig;

    // Row reduction
    for (int i = 0; i < n1; i++) {
        int mn = *std::min_element(cost[i].begin(), cost[i].end());
        for (int j = 0; j < n2; j++) {
            cost[i][j] -= mn;
        }
    }

    // Column reduction
    for (int j = 0; j < n2; j++) {
        int mn = INT_MAX;
        for (int i = 0; i < n1; i++) {
            mn = std::min(mn, cost[i][j]);
        }
        for (int i = 0; i < n1; i++) {
            cost[i][j] -= mn;
        }
    }

    std::vector<int> match(n1, -1);

    while (true) {
        std::vector<int> set2(n2, -1);
        std::fill(match.begin(), match.end(), -1);

        for (int i = 0; i < n1; i++) {
            std::vector<int> visited(n1, 0);
            helper(cost, set2, visited, match, i);
        }

        int count = 0;
        for (int j = 0; j < n2; j++) {
            if (set2[j] != -1) count++;
        }
        if (count == n1) break;

        std::vector<int> row_covered(n1, 0);
        std::vector<int> col_covered(n2, 0);
        mark(cost, match, set2, row_covered, col_covered);

        std::vector<int> final_row(n1), final_col(n2);
        for (int i = 0; i < n1; i++) final_row[i] = !row_covered[i];
        for (int j = 0; j < n2; j++) final_col[j] = col_covered[j];

        int mn = INT_MAX;
        for (int i = 0; i < n1; i++) {
            for (int j = 0; j < n2; j++) {
                if (!final_row[i] && !final_col[j]) {
                    mn = std::min(mn, cost[i][j]);
                }
            }
        }
        if (mn == INT_MAX) break;

        for (int i = 0; i < n1; i++) {
            for (int j = 0; j < n2; j++) {
                if (!final_row[i] && !final_col[j])
                    cost[i][j] -= mn;
                else if (final_row[i] && final_col[j])
                    cost[i][j] += mn;
            }
        }
    }

    return match;
}