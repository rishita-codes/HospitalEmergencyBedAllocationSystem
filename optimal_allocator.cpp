#include "optimal_allocator.h"
#include "database.h"
#include "bipartite_matching.h"  
#include <vector>
#include <queue>
#include <algorithm>
#include <climits>
#include <iostream>
#include <functional>     
OptimalAllocator::OptimalAllocator(std::vector<Patient*> p, std::vector<Bed>& b)
    : patients(p), beds(b)
{
    freeBedsIndex.clear();
    for (int i = 0; i < beds.size(); i++) {
        if (!beds[i].occupied()) {
            freeBedsIndex.push_back(i);
        }
    }
}

bool OptimalAllocator::isCompatible(BedType b, ESI e){
    if (e == ESI1 && b == ICU) return true;
    if (e == ESI2 && (b == ICU || b == EMERGENCY)) return true;
    if (e == ESI3 && (b == GENERAL || b == EMERGENCY)) return true;
    if (e == ESI4) return true;
    return false;
}

void OptimalAllocator::buildCostMatrix(){
    int n = patients.size();
    int m = freeBedsIndex.size();

    costMatrix.assign(n, std::vector<int>(m, 0));

    for(int i = 0; i < n; i++){
        for(int j = 0; j < m; j++){
            BedType bt = beds[freeBedsIndex[j]].getType();

            if(!isCompatible(bt, patients[i]->getESI())){
                costMatrix[i][j] = 100000;
            } else {
                int priority = patients[i]->calculatePriority();
                costMatrix[i][j] = -priority;  
            }
        }
    }
}

std::vector<int> OptimalAllocator::hungarian(std::vector<std::vector<int>> cost){
    int n1 = cost.size();
    int n2 = n1 > 0 ? cost[0].size() : 0;

    if (n1 == 0 || n2 == 0) {
        return std::vector<int>(n1, -1);
    }

    for(int i = 0; i < n1; i++){
        int mn = *std::min_element(cost[i].begin(), cost[i].end());
        for(int j = 0; j < n2; j++){
            cost[i][j] -= mn;
        }
    }

    for(int j = 0; j < n2; j++){
        int mn = INT_MAX;
        for(int i = 0; i < n1; i++){
            mn = std::min(mn, cost[i][j]);
        }
        for(int i = 0; i < n1; i++){
            cost[i][j] -= mn;
        }
    }

    std::vector<int> match(n1, -1);

    while(true){
        std::vector<int> set2(n2, -1);

        std::function<bool(int, std::vector<int>&)> dfs = [&](int u, std::vector<int>& vis){
            if(vis[u]) return false;
            vis[u] = 1;

            for(int v = 0; v < n2; v++){
                if(cost[u][v] == 0){
                    if(set2[v] == -1 || dfs(set2[v], vis)){
                        match[u] = v;
                        set2[v] = u;
                        return true;
                    }
                }
            }
            return false;
        };

        std::fill(match.begin(), match.end(), -1);

        for(int i = 0; i < n1; i++){
            std::vector<int> vis(n1, 0);
            dfs(i, vis);
        }

        int cnt = 0;
        for(int j = 0; j < n2; j++){
            if(set2[j] != -1) cnt++;
        }

        if(cnt == n1) break;

        std::vector<int> row(n1, 0), col(n2, 0);
        std::queue<int> q;

        for(int i = 0; i < n1; i++){
            if(match[i] == -1){
                q.push(i);
                row[i] = 1;
            }
        }

        while(!q.empty()){
            int r = q.front(); q.pop();

            for(int c = 0; c < n2; c++){
                if(cost[r][c] == 0 && !col[c]){
                    col[c] = 1;
                    if(set2[c] != -1 && !row[set2[c]]){
                        row[set2[c]] = 1;
                        q.push(set2[c]);
                    }
                }
            }
        }

        std::vector<int> fr(n1, 0), fc(n2, 0);
        for(int i = 0; i < n1; i++) fr[i] = !row[i];
        for(int j = 0; j < n2; j++) fc[j] = col[j];

        int mn = INT_MAX;
        for(int i = 0; i < n1; i++){
            for(int j = 0; j < n2; j++){
                if(!fr[i] && !fc[j]){
                    mn = std::min(mn, cost[i][j]);
                }
            }
        }

        if(mn == INT_MAX) break;

        for(int i = 0; i < n1; i++){
            for(int j = 0; j < n2; j++){
                if(!fr[i] && !fc[j]){
                    cost[i][j] -= mn;
                } else if(fr[i] && fc[j]){
                    cost[i][j] += mn;
                }
            }
        }
    }

    return match;
}

void OptimalAllocator::computeOptimal(){
    buildCostMatrix();
    match = hungarian(costMatrix);
}

void OptimalAllocator::showAllocation(){
    std::cout << "\nOptimal Allocation Plan:\n";

    bool anyAssigned = false;
    for(int i = 0; i < match.size(); i++){
        if(match[i] != -1){
            int bedIdx = freeBedsIndex[match[i]];
            std::cout << "Patient " << patients[i]->getId()
                      << " -> Bed " << beds[bedIdx].getBedId()
                      << " (type=" << beds[bedIdx].getType() << ")\n";
            anyAssigned = true;
        }
    }

    if (!anyAssigned) {
        std::cout << "No beds available or no compatible assignment found.\n";
    }
}

void OptimalAllocator::applyAllocation(Database* db){
    for(int i = 0; i < match.size(); i++){
        if(match[i] != -1){
            int bedIdx = freeBedsIndex[match[i]];
            std::string id = patients[i]->getId();
            beds[bedIdx].assignPatient(id);
            if (db) {
                db->assignBed(id, beds[bedIdx].getBedId());
            }
        }
    }
}
const std::vector<int>& OptimalAllocator::getMatch() const {
    return match;
}
