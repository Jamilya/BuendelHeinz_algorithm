#include <iostream>
#include <bits/stdc++.h>
#include <fstream>
#include <sstream>

using namespace std;

struct Node
{
    int requestID;
    int pickupNodeID;
    int deliveryNodeID;
    int releaseTime;
    int dueTime;
};

struct IndexData
{
    int originalID;
    int currentID;
};

const int originalCountOfRequests = 3;
int countOfRequests = 3;
int final_path[originalCountOfRequests * 2 + 2];
bool visited[originalCountOfRequests * 2 + 1];

int final_res = INT_MAX;
int adj[originalCountOfRequests * 2 + 1][originalCountOfRequests * 2 + 1];
Node requests[originalCountOfRequests];
IndexData requestCostMatches[originalCountOfRequests * 2];

void copyToFinal(int curr_path[])
{
    for (int i = 0; i < countOfRequests * 2 + 1; i++)
        final_path[i] = curr_path[i];
    final_path[countOfRequests * 2 + 1] = curr_path[0];
}

int firstMin(int adj[originalCountOfRequests * 2 + 1][originalCountOfRequests * 2 + 1], int i)
{
    int min = INT_MAX;
    for (int k = 0; k < countOfRequests * 2 + 1; k++)
        if (adj[i][k] < min && i != k)
            min = adj[i][k];
    return min;
}

int secondMin(int adj[originalCountOfRequests * 2 + 1][originalCountOfRequests * 2 + 1], int i)
{
    int first = INT_MAX, second = INT_MAX;
    for (int j = 0; j < countOfRequests * 2 + 1; j++)
    {
        if (i == j)
            continue;

        if (adj[i][j] <= first)
        {
            second = first;
            first = adj[i][j];
        }
        else if (adj[i][j] <= second && adj[i][j] != first)
            second = adj[i][j];
    }
    return second;
}

void TSPRec(int adj[originalCountOfRequests * 2 + 1][originalCountOfRequests * 2 + 1], int curr_bound, int curr_weight,
            int level, int curr_path[])
// void TSPRec(int adj[N][N], vector<Node> nodes[N], int curr_bound, int curr_weight,
//             int level, int curr_path[])
{
    if (level == countOfRequests * 2 + 1)
    {
        if (adj[curr_path[level - 1]][curr_path[0]] != 0)
        {
            int curr_res = curr_weight + adj[curr_path[level - 1]][curr_path[0]];
            if (curr_res < final_res)
            {
                copyToFinal(curr_path);
                final_res = curr_res;
            }
        }
        return;
    }

    for (int i = 0; i < countOfRequests * 2 + 1; i++)
    {
        if (adj[curr_path[level - 1]][i] != 0 && visited[i] == false)
        {
            int temp = curr_bound;
            curr_weight += adj[curr_path[level - 1]][i];

            if (level == 1)
                curr_bound -= ((firstMin(adj, curr_path[level - 1]) + firstMin(adj, i)) / 2);
            else
                curr_bound -= ((secondMin(adj, curr_path[level - 1]) + firstMin(adj, i)) / 2);

            if (curr_bound + curr_weight < final_res)
            {
                curr_path[level] = i;
                visited[i] = true;
                TSPRec(adj, curr_bound, curr_weight, level + 1, curr_path);
            }

            curr_weight -= adj[curr_path[level - 1]][i];
            curr_bound = temp;
            memset(visited, false, sizeof(visited));
            for (int j = 0; j <= level - 1; j++)
                visited[curr_path[j]] = true;
        }
    }
}

void TSP (int adj[originalCountOfRequests * 2 + 1][originalCountOfRequests * 2 + 1])
{
    int curr_path[countOfRequests * 2 + 2];
    int curr_bound = 0;
    memset(curr_path, -1, sizeof(curr_path));
    memset(visited, 0, sizeof(visited));

    for (int i = 0; i < countOfRequests * 2 + 1; i++)
        curr_bound += (firstMin(adj, i) + secondMin(adj, i));

    curr_bound = (curr_bound & 1) ? curr_bound / 2 + 1 : curr_bound / 2;

    visited[0] = true;
    curr_path[0] = 0;
    TSPRec(adj, curr_bound, 0, 1, curr_path);
}


void reindexFollowUpNodes(int id) {
    //todo link original indizes
    for (int i = id; i < originalCountOfRequests * 2; i++) {
        if (requestCostMatches[i].currentID == -1)
            continue;
        requestCostMatches[i].currentID--;
    }
}

void reindexMatchingTable(int id) {
    requestCostMatches[id - 1].currentID = -1;
    reindexFollowUpNodes(id);
}

void parseCosts(string path)
{
    ifstream file(path);
    if (!file.is_open())
    {
        std::cout << "Error opening plop.csv!" << std::endl;
        return;
    }

    string line;
    int row = 0;
    while (getline(file, line))
    {
        stringstream ss(line);
        string cell;
        int col = 0;
        while (getline(ss, cell, ','))
        {
            adj[row][col] = stoi(cell);
            col++;
        }
        row++;
    }
    file.close();
}

void parseRequests(string path)
{
    ifstream nodesFile(path);
    if (!nodesFile.is_open())
    {
        std::cout << "Error opening nodes.csv!" << std::endl;
        return;
    }
    int row = 0;
    string line;
    while (getline(nodesFile, line))
    {
        stringstream ss(line);
        string cell;
        int col = 0;
        while (getline(ss, cell, ','))
        {
            if (col == 0)
                requests[row].requestID = stoi(cell);
            else if (col == 1)
                requests[row].pickupNodeID = stoi(cell);
            else if (col == 2)
                requests[row].deliveryNodeID = stoi(cell);
            else if (col == 3)
                requests[row].releaseTime = stoi(cell);
            else if (col == 4)
                requests[row].dueTime = stoi(cell);
            col++;
        }
        row++;
    }
    nodesFile.close();
}

void setupMatchingTable() {
    //no garbage collector.. clean up the previous ids!! (?)
    for (int i = 0; i < originalCountOfRequests * 2; i++) {
        IndexData id;
        id.currentID = i + 1;
        id.originalID = i + 1;
        requestCostMatches[i] = id;
    }
}

void validateAndBuildRequestCluser(int cluster,  vector<Node>& filteredRequests) {
    bool addNode = false;

    for (int i = 0; i < countOfRequests; i++)
    {
        addNode = false;
            
        // if current row belongs to current cluster, add it to array --> and then hand the array over to the function the finds the best Path
        if (requests[i].releaseTime == cluster && requests[i].releaseTime <= requests[i].dueTime &&
            requests[i].pickupNodeID < requests[i].deliveryNodeID)
            addNode = true;

        if (addNode)
            filteredRequests.push_back(requests[i]);
        else {
            reindexMatchingTable(requests[i].pickupNodeID);
            reindexMatchingTable(requests[i].deliveryNodeID);
        }
    }
}

void createFilteredCostMatrixByClusteredRequests(int filteredAdj[][originalCountOfRequests * 2 + 1]) {
    int decreaseIndexCount = 0;
    for (int i = 0; i < originalCountOfRequests * 2 + 1; i++) {
        int decreaseIndexCountColumn = 0;
        if (i == 0 || requestCostMatches[i - 1].currentID != -1) {
            for (int j = 0; j < originalCountOfRequests * 2 + 1; j++) {
                if (j == 0 || requestCostMatches[j - 1].currentID != -1)
                    filteredAdj[i - decreaseIndexCount][j - decreaseIndexCountColumn] = adj[i][j];
                else
                    decreaseIndexCountColumn++;
            }
        }
        else
            decreaseIndexCount++;
    }
}

void printResults(int cluster) {
    std::cout << "Cluster: " << cluster << std::endl;
    std::cout << "Minimum cost: " << final_res << std::endl;
    std::cout << "Path Taken new: " << std::endl;;
    for (int i = 0; i <= countOfRequests * 2 + 1; i++)
        std::cout << final_path[i] << std::endl;

    std::cout << "Path Taken original: " << std::endl;
    string finalString = "";
    for (int i = 0; i <= countOfRequests * 2 + 1; i++) {
        if (final_path[i] == 0) {
            finalString += std::to_string(0) + "\n";
            continue;
        }
        for (int j = 0; j <= originalCountOfRequests * 2; j++) 
            if (requestCostMatches[j].currentID == final_path[i]) 
                finalString += std::to_string(requestCostMatches[j].originalID) + "\n";
    }
    std::cout << finalString << std::endl;
}

void calculateAndPrintBestRouteForCluster(int cluster)
{
    int filteredAdj[originalCountOfRequests * 2 + 1][originalCountOfRequests * 2 + 1];
    vector<Node> filteredRequests;
    validateAndBuildRequestCluser(cluster, filteredRequests);
    countOfRequests = filteredRequests.size();
    if (countOfRequests == 0)
        return;
    createFilteredCostMatrixByClusteredRequests(filteredAdj);
   
    TSP(filteredAdj);
    printResults(cluster);
}

void resetData() {
    for (int i = 0; i <= countOfRequests; i++)  {
        final_path[i] = -1;
        visited[i] = false;
    }
    final_res = INT_MAX;
    countOfRequests = originalCountOfRequests;
}

int main()
{
    setupMatchingTable();
    parseCosts("plop.csv");
    parseRequests("requests.csv");
    // vector<Node> nodes[N];
    int clusterCount = 1;
    for (int i = 0; i < originalCountOfRequests; i++)
    {
        if (requests[i].releaseTime + 1 > clusterCount)
            clusterCount = requests[i].releaseTime + 1;
    }

    for (int i = 0; i < clusterCount; i++)
    {
        setupMatchingTable();
        resetData();
        calculateAndPrintBestRouteForCluster(i);
    }
}






/*class PossibleRoute {
    Node[] nodesInRoute;
    int costOfRoute;
}*/