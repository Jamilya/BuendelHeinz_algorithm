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
//int final_path[countOfRequests * 2 + 2];
//bool visited[countOfRequests * 2 + 1];
int *final_path;
bool *visited;
int final_res = INT_MAX;
int adj[originalCountOfRequests * 2 + 1][originalCountOfRequests * 2 + 1];
Node requests[originalCountOfRequests];
//int requestCostMatches[N * 2];
IndexData requestCostMatches[originalCountOfRequests];


void copyToFinal(int curr_path[])
{
    for (int i = 0; i < countOfRequests * 2 + 1; i++)
        final_path[i] = curr_path[i];
    final_path[countOfRequests * 2 + 1] = curr_path[0];
}

int firstMin(int size, int adj[size][size])
{
    int min = INT_MAX;
    for (int k = 0; k < countOfRequests * 2 + 1; k++)
        if (adj[i][k] < min && i != k)
            min = adj[i][k];
    return min;
}

int secondMin(int size, int adj[size * 2 + 1][size * 2 + 1], int i)
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

void TSPRec(int size, int adj[size * 2 + 1][size * 2 + 1], int curr_bound, int curr_weight,
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
                curr_bound -= ((firstMin(size, adj, curr_path[level - 1]) + firstMin(adj, i)) / 2);
            else
                curr_bound -= ((secondMin(size, adj, curr_path[level - 1]) + firstMin(adj, i)) / 2);

            if (curr_bound + curr_weight < final_res)
            {
                curr_path[level] = i;
                visited[i] = true;
                TSPRec(size, adj, curr_bound, curr_weight, level + 1, curr_path);
            }

            curr_weight -= adj[curr_path[level - 1]][i];
            curr_bound = temp;
            memset(visited, false, sizeof(visited));
            for (int j = 0; j <= level - 1; j++)
                visited[curr_path[j]] = true;
        }
    }
}

void TSP (int size, int adj[size * 2 + 1][size * 2 + 1])
// void TSP(int adj[N][N], vector<Node> nodes[N])
{
    countOfRequests = size;
    final_path = new int[countOfRequests * 2 + 2];
    visited = new bool[countOfRequests * 2 + 1];

    int curr_path[countOfRequests * 2 + 2];
    int curr_bound = 0;
    memset(curr_path, -1, sizeof(curr_path));
    memset(visited, 0, sizeof(visited));

    for (int i = 0; i < countOfRequests * 2 + 1; i++)
        curr_bound += (firstMin(adj, i) + secondMin(adj, i));

    curr_bound = (curr_bound & 1) ? curr_bound / 2 + 1 : curr_bound / 2;

    visited[0] = true;
    curr_path[0] = 0;

    TSPRec(size, adj, curr_bound, 0, 1, curr_path);
}

void GetBestRouteForCluster(int cluster)
{
    vector<Node> filteredRequests;
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

    int filteredAdj[filteredRequests.size() * 2 + 1][filteredRequests.size() * 2 + 1];

    int decreaseIndexCount = 0;

    for (int i = 0; i < countOfRequests; i++) {
        int decreaseIndexCountColumn = 0;
        if (i == 0 || requestCostMatches[i - 1].currentID != -1) {
            for (int j = 0; j < countOfRequests; j++) {
                if (j == 0 || requestCostMatches[j - 1].currentID != -1){
                    filteredAdj[i - decreaseIndexCount][j - decreaseIndexCountColumn] = adj[i][j];                    
                }
                else
                    decreaseIndexCountColumn++;
            }
        }
    
        else
            decreaseIndexCount++;
    }
    /*for (int i = 0; i <filteredNodes.size() * 2 + 1; i++)
    {
        filteredAdj[i] = adj[filteredNodes[i].pickupNodeID];
       //for (int j = 0; j < N; j++)
       // {
       //     filteredAdj[filteredNodes[i].pickupNodeID][filteredNodes[j].pickupNodeID] = adj[filteredNodes[i].pickupNodeID][filteredNodes[j].pickupNodeID];
      //  }
    }*/
    // TSP(filteredAdj, filteredNodes);
    TSP(filteredRequests.size(), filteredAdj);

  /*  for (int i = 0; i < filteredNodes.size(); i++)
    {
        for (int j = 0; j < filteredNodes.size() * 2; j++)
        {
            filteredAdj[filteredRequests[i].pickupNodeID] = 
            
            [filteredRequests[j].pickupNodeID] = adj[filteredRequests[i].pickupNodeID][filteredRequests[j].pickupNodeID];
        }
    }*/

   // TSP(adj, filteredRequests.data());

    std::cout << "Cluster: " << cluster << std::endl;
    std::cout << "Minimum cost: " << final_res << std::endl;
    std::cout << "Path Taken new: ";
    for (int i = 0; i <= countOfRequests; i++)
        std::cout << final_path[i] << std::endl;


    std::cout << "Path Taken original: ";
    string finalString = "";
    /*for (int i = 0; i <= final_path.si; i++) {
        if ()
    }*/

    std::cout << finalString << std::endl;

    delete [] final_path;
    delete [] visited;
}

int main()
{
    setupMatchingTable()

    parseCosts("plop.csv");
    parseRequests("requests.csv");

    // Node nodes[N];
    // vector<Node> nodes[N];

    int clusterCount = 1;
    for (int i = 0; i < originalCountOfRequests; i++)
    {
        if (requests[i].releaseTime + 1 > clusterCount)
            clusterCount = requests[i].releaseTime + 1;
    }

    for (int i = 0; i < clusterCount; i++)
    {
        countOfRequests = originalCountOfRequests;
        GetBestRouteForCluster(i);
    }
    return 0;
    // Filter nodes based on constraints
    vector<Node> filteredRequests;
 
    // Node filteredNodes;
    /*for (int i = 0; i < N; i++)
    {
        for (int j = 0; j < N; j++)
        {
            if (requests[i].releaseTime <= requests[i].dueTime &&
                requests[i].pickupNodeID < requests[i].deliveryNodeID)
            {
                filteredNodes.push_back(requests[i]);
            }
        }
    }*/

    for (int i = 0; i < countOfRequests; i++)
    {
        if (requests[i].releaseTime <= requests[i].dueTime &&
            requests[i].pickupNodeID < requests[i].deliveryNodeID)
        {
            filteredRequests.push_back(requests[i]);
        }
    }
    // Update N and adj based on filtered nodes
    // N = filteredNodes.size();
    int filteredAdj[filteredNodes.size() * 2 + 1][filteredNodes.size() * 2 + 1];
    for (int i = 0; i <filteredNodes.size() * 2 + 1; i++)
    {
        filteredAdj[i] = adj[filteredNodes[i].pickupNodeID];
       /*for (int j = 0; j < N; j++)
        {
            filteredAdj[filteredNodes[i].pickupNodeID][filteredNodes[j].pickupNodeID] = adj[filteredNodes[i].pickupNodeID][filteredNodes[j].pickupNodeID];
        }*/
    }
    // TSP(filteredAdj, filteredNodes);
    TSP(filteredAdj);

    std::cout << "Minimum cost: " << final_res << std::endl;
    std::cout << "Path Taken: ";
    for (int i = 0; i <= countOfRequests; i++) {
        std::cout << final_path[i] << " ";
    }

    return 0;
}

void parseCosts(string path)
{
    ifstream file(path);
    if (!file.is_open())
    {
        std::cout << "Error opening plop.csv!" << std::endl;
        return 0;
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
        return 0;
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

/*
void reindexNodes(int deliveryNodeId, int pickUpNodeId) {

    //todo link original indizes
    for(int i = 0; i < N; i++) {
        if (requests[i].deliveryNodeID > deliveryNodeId)
            requests[i].deliveryNodeID --;
        if (requests[i].deliveryNodeID > deliveryNodeId)
            requests[i].deliveryNodeID --;
    }
}

void createRequestCostMatch(int startIndex) {
    requestCostMatches[startIndex] = -1;
    for(int i = startIndex; i < N * 2; i++) {
        requestCostMatches[startIndex] -= 1;
    }
}*/

void setupMatchingTable() {
    for (int i = 0; i < countOfRequests; i++) {
        IndexData id = new IndexData();
        id.currentID = i + 1;
        id.originalID = i + 1;
        requestCostMatches[i] = id;
    }
}

void reindexMatchingTable(int id) {
    requestCostMatches[id - 1].currentID = -1;
    reindexFollowUpNodes(id);
}

void reindexFollowUpNodes(int id) {
    //todo link original indizes
    for(int i = id; i < countOfRequests; i++) {
        requestCostMatches[id].currentID--;
    }
}

/*class PossibleRoute {
    Node[] nodesInRoute;
    int costOfRoute;
}*/