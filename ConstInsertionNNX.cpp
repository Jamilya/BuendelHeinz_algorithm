#include <iostream>
#include <bits/stdc++.h>
#include <fstream>
#include <sstream>

using namespace std;

const int originalCountOfRequests = 100;

struct Node
{
    int correspondingRequestId;
    Node* correspondingNode;
    bool isPickupNode;
    bool visited;
    int originalID;
    int currentID;
    //Cluster* currentCluster;
    int currentPositionInCluster;
    int clusterIdInClusterArray;
};

struct Request
{
    int requestID;
    int pickupNodeID;
    int deliveryNodeID;
    int releaseTime;
    int dueTime;
    Node* pickUpNode;
    Node* deliveryNode;
};

struct Cluster
{
    int totalCost;
    Node* nodes[originalCountOfRequests * 2];
    Node* nodesBackup[originalCountOfRequests * 2];
    int clusterDay;
    int subCosts[originalCountOfRequests * 2 + 1];
    int timeId;
    int actualNodeCount = 0;
    int clusterIdInArray;
};

struct LocalSearchData {
    /*int startCostsOfCurrentFocusCluster = 0;
    int currentBestOverAllCosts = 0;*/
    Cluster* clusterOfBestInsertion;
    int nodePositionOfBestInsertion;

    Node* currentInsertionNode;
    Cluster* clusterForInsertion;
    int insertedNodePosition = 0;

    Cluster* currentFocusCluster;
    int currentNodePositionOfFocusCluster = -1;
    int currentFocusClusterIndex = 0;
    //int bestPotentialSaving = 0;
    bool noMoreOptimizationPotential = false;
    bool savingCalculated = false;
    
    int currentFocusClusterReducedCost = 0;
    int currentOverAllSum = 0;
};

int countOfRequests = originalCountOfRequests;
int final_path[originalCountOfRequests * 2 + 2];
int final_res = INT_MAX;
int subCosts[originalCountOfRequests * 2 + 1];
int subCostCounter = 0;
int adj[originalCountOfRequests * 2 + 1][originalCountOfRequests * 2 + 1];

Request requests[originalCountOfRequests];
Node allNodes[originalCountOfRequests * 2];  
Cluster clusters[originalCountOfRequests];
int originalClusterCosts[originalCountOfRequests];
int clusterCounter;
Cluster currentCluster;

LocalSearchData localSearchData;

void reindexFollowUpNodes(int id) {
    for (int i = id; i < originalCountOfRequests * 2; i++) {
        if (allNodes[i].currentID == -1)
            continue;
        allNodes[i].currentID--;
    }
}

void reindexMatchingTable(int id) {
    allNodes[id - 1].currentID = -1;
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
            if (col == 0) {
                requests[row].requestID = stoi(cell);
            }
            else if (col == 1) {
                requests[row].pickupNodeID = stoi(cell);
                /*
                Node pickupNode;
                Node deliveryNode;
                */
                for (int z = 0; z < originalCountOfRequests * 2; z++) {
                    if (allNodes[z].originalID == requests[row].pickupNodeID) {
                        allNodes[z].correspondingRequestId = row;
                        requests[row].pickUpNode = &allNodes[z];
                        requests[row].pickUpNode->isPickupNode = true;
                        break;
                    }
                }
            
            }
            else if (col == 2) {
                requests[row].deliveryNodeID = stoi(cell);
                for (int z = 0; z < originalCountOfRequests * 2; z++) {
                    if (allNodes[z].originalID == requests[row].deliveryNodeID) {
                        allNodes[z].correspondingRequestId= row;
                        requests[row].deliveryNode = &allNodes[z];
                        requests[row].deliveryNode->isPickupNode = false;
                        break;
                    }
                }
            }
            else if (col == 3) {
                requests[row].pickUpNode->correspondingNode = requests[row].deliveryNode;
                requests[row].deliveryNode->correspondingNode = requests[row].pickUpNode;
                requests[row].releaseTime = stoi(cell);
            }
            else if (col == 4)
                requests[row].dueTime = stoi(cell);
            col++;
        }
        row++;
    }
    nodesFile.close();
}

void setupMatchingTable() {
    for (int i = 0; i < originalCountOfRequests * 2; i++) {
        //Node id;
        allNodes[i].currentID = i + 1;
        allNodes[i].originalID = i + 1;
        allNodes[i].visited = false;
    }
}

void validateAndBuildRequestCluster(int cluster,  vector<Request>& filteredRequests) {
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
        if (i == 0 || allNodes[i - 1].currentID != -1) {
            for (int j = 0; j < originalCountOfRequests * 2 + 1; j++) {
                if (j == 0 || allNodes[j - 1].currentID != -1)
                    filteredAdj[i - decreaseIndexCount][j - decreaseIndexCountColumn] = adj[i][j];
                else
                    decreaseIndexCountColumn++;
            }
        }
        else
            decreaseIndexCount++;
    }
}

//----------------------------------------------------------------- START IMPORTANT FUNCTIONS TO CALL FOR ALGORITHM
bool zeroVisited = false;

void setNodeAsVisited(int currentCostMatrixId) 
{

    if (currentCostMatrixId == 0) {
        zeroVisited = true;
    }

    for (int j = 0; j < originalCountOfRequests * 2; j++) {
        if (allNodes[j].currentID == currentCostMatrixId)
            allNodes[j].visited = true;
    }
}

bool hasNodeBeenVisited(int currentCostMatrixId) {
     if (currentCostMatrixId == 0) 
        return zeroVisited;
    for (int z = 0; z < originalCountOfRequests * 2; z++) {
        if (allNodes[z].currentID == currentCostMatrixId) 
           return allNodes[z].visited;
    }
}

bool isNodePickupNode(int currentCostMatrixId) {
    if (currentCostMatrixId == 0) 
        return false;
    for (int j = 0; j < originalCountOfRequests * 2; j++) {
        if (allNodes[j].currentID == currentCostMatrixId) 
           return allNodes[j].isPickupNode;
    }
}

int getPickupNodeIdOfDeliveryNode(int currentCostMatrixId) {
    if (currentCostMatrixId == 0) 
        return 0;
    if (isNodePickupNode(currentCostMatrixId))
        return -2;
    for (int j = 0; j < originalCountOfRequests * 2; j++) {
        if (allNodes[j].currentID == currentCostMatrixId)
           return allNodes[j].correspondingNode->currentID;
    }
}

int getOriginalId(int currentCostMatrixId) {
    for (int j = 0; j < originalCountOfRequests * 2; j++) {
        if (allNodes[j].currentID == currentCostMatrixId)
           return allNodes[j].originalID;
    }
}

void printVisited() {
    for (int z = 0; z < originalCountOfRequests * 2; z++) {
        //std::cout << "visited: "  << z  << " :" << requestCostMatches[z].visited << std::endl;
    }
}
//----------------------------------------------------------------- END IMPORTANT FUNCTIONS TO CALL FOR ALGORITHM



//----------------------------------------------------------------- START INDIVIDUAL ALGORITHM IMPLEMENTATION

//MODIFY OR IMPLEMENT:
void resetData() {
    for (int i = 0; i <= countOfRequests * 2 + 1 ; i++)  {
        final_path[i] = -1;
        //visited[i] = false;
    }
    final_res = INT_MAX;
    subCostCounter = 0;
    countOfRequests = originalCountOfRequests;
    for (int i = 0; i < countOfRequests * 2 -1; i++)  
        subCosts[i] = 0;    
}

bool isAllowedToBeVisited(int index) {
    bool skipThis = !isNodePickupNode(index) && (!hasNodeBeenVisited(getPickupNodeIdOfDeliveryNode(index)));
    if (skipThis)
        return false;
    return !hasNodeBeenVisited(index);
}

void TSP (int adj[originalCountOfRequests * 2 + 1][originalCountOfRequests * 2 + 1]) {
    memset(final_path, -1, sizeof(final_path));

    for (int n = 0; n < originalCountOfRequests * 2; n++) {
        allNodes[n].visited = false;
    }
    
    zeroVisited = false;
    final_path[0] = 0;
    setNodeAsVisited(0);

    //final_path_count = 1;
    int minAmount = INT32_MAX;
    int indexOfNextNode = 0;
    int indexOfNextNodeTemp = 0;
    int totalAmount = 0;
    Node* nextNode = NULL;
    for (int i = 0; i < countOfRequests * 2; i++) {
        for (int j = 0; j < countOfRequests * 2 + 1; j++) {
             if (indexOfNextNode == j)
                continue;
            if (adj[indexOfNextNode][j] < minAmount && isAllowedToBeVisited(j)) {
                minAmount = adj[indexOfNextNode][j];
                indexOfNextNodeTemp = j;
            }
        }
        indexOfNextNode = indexOfNextNodeTemp;
        setNodeAsVisited(indexOfNextNode);
        final_path[i + 1] = indexOfNextNode;
        subCosts[i] = minAmount;
        totalAmount += minAmount;
        minAmount = INT32_MAX;
    }
    final_path[countOfRequests * 2 + 1] = 0;
    subCosts[countOfRequests * 2] = adj[indexOfNextNode][0];
    totalAmount += adj[indexOfNextNode][0];
    final_res = totalAmount;
}


int sum1 = 0;
//MODIFY OR IMPLEMENT:
void printResults(int cluster) {
    sum1 += final_res;

    std::cout << "CLUSTER FOR DAY " << std::to_string(cluster) << std::endl;
   // std::cout << "Cluster: " << cluster << std::endl;
   // std::cout << "Minimum cost: " << final_res << std::endl;
   /* std::cout << "Path Taken new: " << std::endl;;
    for (int i = 0; i <= countOfRequests * 2 + 1; i++)
        std::cout << final_path[i] << std::endl;
*/
    //std::cout << "Path Taken original: " << std::endl;
    string finalString = "";
    for (int i = 0; i <= countOfRequests * 2 + 1; i++) {
        if (final_path[i] == 0) {
            finalString += std::to_string(0) + "\n";
            continue;
        }
        int originalId = getOriginalId(final_path[i]);
        finalString += std::to_string(originalId) + "\n";
        /*for (int j = 0; j < originalCountOfRequests * 2 ; j++) 
            if (requestCostMatches[j].currentID == final_path[i]) {
                finalString += std::to_string(requestCostMatches[j].originalID) + "\n";
                break;
            }*/
    }
    std::cout << finalString << std::endl;
    std::cout << "SUM: " << final_res << std::endl;


}

Cluster setupClusterData() {
    Cluster buildCluster;
    buildCluster.clusterIdInArray = clusterCounter;
    for (int i = 1; i <= countOfRequests * 2; i++) {
        int originalId = getOriginalId(final_path[i]);
        buildCluster.nodes[i - 1] = &allNodes[originalId - 1];
        buildCluster.actualNodeCount++;
        buildCluster.nodes[i - 1]->clusterIdInClusterArray = buildCluster.clusterIdInArray;
        buildCluster.nodes[i - 1]->currentPositionInCluster = i - 1;
    }
    /*for (int i = 0; i < countOfRequests * 2 + 1; i++)
        buildCluster.subCosts[i] = subCosts[i];*/
    return buildCluster;
}

double median(int arr[], int size) {
   sort(arr, arr+size);
   if (size % 2 != 0)
      return (double)arr[size/2];
   return (double)(arr[(size-1)/2] + arr[size/2])/2.0;
}

void checkCalculatedCluster() {
    currentCluster = setupClusterData();
    //int medianCost = median(subCosts, originalCountOfRequests * 2 + 1);
    //todo run through currentCluster.subCosts... if some Costs are > medianCost * 4.3, then and nextClusterNumber is smaller then Nodes deliveryDate add Node to drop list. Then parse droplist
    //however: Still 
}

int getNodeToBaseCost(Node* node) {
    if (node == NULL)
        return 0;
    return adj[node->originalID][0];
}

int getEdgeCost(Node* sourceNode, Node* targetNode) {
    if (sourceNode == NULL)
        return getNodeToBaseCost(targetNode);
    if (targetNode == NULL)
        return getNodeToBaseCost(sourceNode);
    return adj[sourceNode->originalID][targetNode->originalID];
}

int calculateCostsForCluster(Cluster* cluster) {
    int calculatedCosts = 0;
    Node* nodeToGoTo;
    Node* nodeToComeFrom;
    int offset = 0;
    int u = 0;
    int h = 0;
   // int zuzu[9];
    int calls = 0;
    bool igo = false;
    for (int i = -1; i < cluster->actualNodeCount; i++) {
        calls++;
        if (i + offset == -1)
            nodeToComeFrom = NULL;
        else 
            nodeToComeFrom = cluster->nodes[i + offset];

        if (i + offset + 1 == cluster->actualNodeCount)
            nodeToGoTo = NULL;
        else 
            nodeToGoTo = cluster->nodes[i + 1 + offset];

        if (localSearchData.clusterForInsertion == cluster && localSearchData.insertedNodePosition == i + offset && !igo) {
            nodeToGoTo = cluster->nodes[i + offset];
            nodeToComeFrom = localSearchData.currentInsertionNode;
            offset--;
            igo = true;
        }
      
        else if (localSearchData.clusterForInsertion == cluster && localSearchData.insertedNodePosition == i + 1 && offset < 1 && !igo) {
            nodeToGoTo = localSearchData.currentInsertionNode;
            offset++;
            i--;
        }
        //zuzu[u] = getEdgeCost(nodeToComeFrom, nodeToGoTo);
        //u++;
        calculatedCosts += getEdgeCost(nodeToComeFrom, nodeToGoTo);
        h = calculatedCosts;

    }
    return calculatedCosts;
}

void calculateCurrentCostsPerCluster() {
    Cluster* tempPointerToInsertionCluster = localSearchData.clusterForInsertion;
    localSearchData.clusterForInsertion = NULL;
    localSearchData.currentOverAllSum = 0;
    for (int k = 0; k < clusterCounter; k++) {
        originalClusterCosts[k] = calculateCostsForCluster(&clusters[k]);
        localSearchData.currentOverAllSum += originalClusterCosts[k];
    }
    localSearchData.clusterForInsertion = tempPointerToInsertionCluster;
}

void insertionIteration() {
    //int bestSavingOverAllClusters = 0;
    /*int currentIterationBestPrice = -1;
    int currentBestCostsPerCluster[clusterCounter];
    for (int i = 0; i < clusterCounter; i++) {
        currentBestCostsPerCluster[i] = -1;//originalClusterCosts[i];
    }*/
    int currentIterationBestPrice = -1;
    for (int i = 0; i < clusterCounter; i++) {
      
        //if (!localSearchData.currentInsertionNode->isPickupNode && (clusters[localSearchData.currentInsertionNode->correspondingNode->clusterIdInClusterArray].clusterDay > clusters[localSearchData.currentInsertionNode->clusterIdInClusterArray].clusterDay))
        if (!localSearchData.currentInsertionNode->isPickupNode && (clusters[localSearchData.currentInsertionNode->correspondingNode->clusterIdInClusterArray].clusterDay > clusters[i].clusterDay))
            continue;
       // if (localSearchData.currentInsertionNode->isPickupNode && (clusters[localSearchData.currentInsertionNode->correspondingNode->clusterIdInClusterArray].clusterDay < clusters[localSearchData.currentInsertionNode->clusterIdInClusterArray].clusterDay))
        if (localSearchData.currentInsertionNode->isPickupNode && (clusters[localSearchData.currentInsertionNode->correspondingNode->clusterIdInClusterArray].clusterDay < clusters[i].clusterDay))
            continue;
        if (requests[localSearchData.currentInsertionNode->correspondingRequestId].releaseTime > clusters[i].clusterDay)
            continue;
        if (requests[localSearchData.currentInsertionNode->correspondingRequestId].dueTime < clusters[i].clusterDay)
            continue;
        localSearchData.clusterForInsertion = &clusters[i];
        int currentClusterCost = -1;
        for (int j = 0; j < localSearchData.clusterForInsertion->actualNodeCount; j++) {
            //if (!localSearchData.currentInsertionNode->isPickupNode && (&clusters[localSearchData.currentInsertionNode->correspondingNode->clusterIdInClusterArray] == &clusters[localSearchData.currentInsertionNode->clusterIdInClusterArray] && localSearchData.currentInsertionNode->correspondingNode->currentPositionInCluster >= j))
            if (!localSearchData.currentInsertionNode->isPickupNode && (&clusters[localSearchData.currentInsertionNode->correspondingNode->clusterIdInClusterArray] == &clusters[i] && localSearchData.currentInsertionNode->correspondingNode->currentPositionInCluster >= j))
                continue;
            //if (localSearchData.currentInsertionNode->isPickupNode && (&clusters[localSearchData.currentInsertionNode->correspondingNode->clusterIdInClusterArray] == &clusters[localSearchData.currentInsertionNode->clusterIdInClusterArray] && localSearchData.currentInsertionNode->correspondingNode->currentPositionInCluster < j))
            if (localSearchData.currentInsertionNode->isPickupNode && (&clusters[localSearchData.currentInsertionNode->correspondingNode->clusterIdInClusterArray] == &clusters[i] && localSearchData.currentInsertionNode->correspondingNode->currentPositionInCluster < j))
                continue;
            localSearchData.insertedNodePosition = j;
            int newCost = calculateCostsForCluster(&clusters[i]);
            if (newCost < currentClusterCost || currentClusterCost == -1) {
                currentClusterCost = newCost;
                int sum = currentClusterCost;
                //if (localSearchData.insertedNodePosition == localSearchData.currentFocusCluster) 
            
                for (int n = 0; n < clusterCounter; n++) {
                    if (n == i)
                        continue;
                    if (&clusters[n] == localSearchData.currentFocusCluster)
                        sum += localSearchData.currentFocusClusterReducedCost;
                    else
                        sum += originalClusterCosts[n];
                }
                if (sum < localSearchData.currentOverAllSum && (sum < currentIterationBestPrice || currentIterationBestPrice == -1)) {
                    currentIterationBestPrice = sum;
                    localSearchData.clusterOfBestInsertion = &clusters[i];
                    localSearchData.nodePositionOfBestInsertion = j;
                }

                //currentBestCostsPerCluster[i] = currentClusterCost;
                /*if (originalClusterCosts[i] - currentClusterCost > bestSavingOverAllClusters) {
                    bestSavingOverAllClusters = originalClusterCosts[i] - currentClusterCost;
                    //if (bestSavingOverAllClusters > localSearchData.bestPotentialSaving) {
                       
                    //}

                }*/
            }
        }
        //currentBestCostsPerCluster[i] plus Kosten aller anderen Cluster (originalClusterCosts) außer dem FokusCluster --> dort reducedCosts.. es sei denn, clusterForInsertion ist der Fokuscluster. Dann currentBestCostsPerCluster[i] plus plus Kosten aller anderen Cluster (originalClusterCosts)
   
    }    
    //localSearchData.bestPotentialSaving = bestSavingOverAllClusters;
    localSearchData.savingCalculated = currentIterationBestPrice != -1;

}

void startNewFocusCluster() {
    for (int i = 0; i < originalCountOfRequests * 2; i++) {
        localSearchData.currentFocusCluster->nodesBackup[i] = localSearchData.currentFocusCluster->nodes[i];
    }
    localSearchData.currentNodePositionOfFocusCluster = -1;
    //setNewInsertionNode();
}

//int called = 0;
void removeNodeFromFocusCluster() {
   //called++;
    int j = 0;
    for (int i = 0; i < localSearchData.currentFocusCluster->actualNodeCount; i++) {
        if (localSearchData.currentInsertionNode != localSearchData.currentFocusCluster->nodes[i]) {
            localSearchData.currentFocusCluster->nodes[j] = localSearchData.currentFocusCluster->nodes[i];
            localSearchData.currentFocusCluster->nodes[j]->currentPositionInCluster = j;
            j++;
        }
    }
    localSearchData.currentFocusCluster->actualNodeCount = j;
    Cluster* tempPointerToInsertionCluster = localSearchData.clusterForInsertion;
    localSearchData.clusterForInsertion = NULL;
    localSearchData.currentFocusClusterReducedCost = calculateCostsForCluster(localSearchData.currentFocusCluster);
    localSearchData.clusterForInsertion = tempPointerToInsertionCluster;
    //localSearchData.insertedNodePosition = 0;
    //localSearchData.clusterForInsertion = clusters[0]:
}

void restoreFocusCluster() { 
    for (int i = 0; i < originalCountOfRequests * 2; i++) {
        localSearchData.currentFocusCluster->nodes[i] = localSearchData.currentFocusCluster->nodesBackup[i];
        if (localSearchData.currentFocusCluster->nodes[i] == NULL)
            continue;
        localSearchData.currentFocusCluster->nodes[i]->currentPositionInCluster = i;
        localSearchData.currentFocusCluster->nodes[i]->clusterIdInClusterArray = localSearchData.currentFocusCluster->clusterIdInArray;
    }
    localSearchData.currentFocusCluster->actualNodeCount++;
    int z = 0;
    z++;
    //setNewInsertionNode();
}

void setNewInsertionNode(bool ignoreRestore) { 
    if (!ignoreRestore)
        restoreFocusCluster();
    localSearchData.currentNodePositionOfFocusCluster++;
    if (localSearchData.currentNodePositionOfFocusCluster == localSearchData.currentFocusCluster->actualNodeCount) {
       // restoreFocusCluster();
        localSearchData.currentFocusClusterIndex++;
    	if (localSearchData.currentFocusClusterIndex == clusterCounter) {
            localSearchData.noMoreOptimizationPotential = true;
        }
        else {
            localSearchData.currentFocusCluster = &clusters[localSearchData.currentFocusClusterIndex];
            startNewFocusCluster();
            setNewInsertionNode(true);
        }
        return;
    }
    localSearchData.currentInsertionNode = localSearchData.currentFocusCluster->nodes[localSearchData.currentNodePositionOfFocusCluster];
    removeNodeFromFocusCluster();
    insertionIteration();
}

 void rebuildInsertionCluster() {
    Node* overwrittenNode;
    Node* overwrittenNode2;
    for (int i = 0; i <= localSearchData.clusterOfBestInsertion->actualNodeCount; i++) {
        if (i == localSearchData.nodePositionOfBestInsertion) {
            overwrittenNode = localSearchData.clusterOfBestInsertion->nodes[i];
            localSearchData.currentInsertionNode->clusterIdInClusterArray = localSearchData.clusterOfBestInsertion->clusterIdInArray;
            localSearchData.clusterOfBestInsertion->nodes[i] = localSearchData.currentInsertionNode;
            localSearchData.clusterOfBestInsertion->nodes[i]->currentPositionInCluster = i;
             for (int j = i + 1; j <= localSearchData.clusterOfBestInsertion->actualNodeCount; j++) {
                overwrittenNode2 = localSearchData.clusterOfBestInsertion->nodes[j]; 
                localSearchData.clusterOfBestInsertion->nodes[j] = overwrittenNode;
                localSearchData.clusterOfBestInsertion->nodes[j]->currentPositionInCluster = j;
                overwrittenNode = overwrittenNode2;
             }
             break;
        }
       /*else {
            localSearchData.clusterOfBestInsertion->nodes[i] = localSearchData.clusterOfBestInsertion->nodes[j];
        }*/
    }
    localSearchData.clusterOfBestInsertion->actualNodeCount++;
 }

void startNextCheckIterartion(bool restart) {// if we tested our node against all positions and it didn't get better restart is false.. If it got better, rest,art is true we call "rebuildCluster" and start the chain again.
    if (restart) {
        rebuildInsertionCluster();
        calculateCurrentCostsPerCluster();
        localSearchData.currentFocusClusterIndex = 0;
        localSearchData.currentFocusCluster = &clusters[localSearchData.currentFocusClusterIndex];
        startNewFocusCluster();
        setNewInsertionNode(true);   
    }
    else {
        setNewInsertionNode(false);
    }
}

int sum2 = 0;
void doOptimizationSteps() {
    //DO LOCAL SEARCH;
    //int potentialSaving = localSearchData.bestPotentialSaving;
    calculateCurrentCostsPerCluster();
    localSearchData.currentFocusClusterIndex = 0;
    localSearchData.currentFocusCluster = &clusters[localSearchData.currentFocusClusterIndex];
    startNewFocusCluster();
    setNewInsertionNode(true);   
    while (!localSearchData.noMoreOptimizationPotential) {
        //if (potentialSaving == localSearchData.bestPotentialSaving) {
        if (localSearchData.savingCalculated) {
            localSearchData.savingCalculated = false;
            startNextCheckIterartion(true);
        }
        else {
            //potentialSaving = localSearchData.bestPotentialSaving;
            startNextCheckIterartion(false);
        }
    }
    std::cout << "IMPROVEN CLUSTERS BY LOCAL SEARCH & INSERTION:" << std::endl;
    localSearchData.clusterForInsertion = NULL;

    for (int k = 0; k < clusterCounter; k++) {
        int sumz = 0;
        sumz = calculateCostsForCluster(&clusters[k]);
        std::cout << "IMPROVED CLUSTER FOR DAY " << std::to_string(clusters[k].clusterDay) << std::endl;
        std::cout << "0" << std::endl;
        for (int m = 0;  m < clusters[k].actualNodeCount; m++) {
            std::cout << std::to_string(clusters[k].nodes[m]->originalID) << std::endl;

        }
        std::cout << "0" << std::endl;

        std::cout << "SUM: " << sumz << std::endl;
        sum2 += sumz;
    }
    std::cout << "COMPARE TOTAL SUM INITIAL CLUSTERING VS. IMPROVED RESULT:" << std::endl;
    std::cout << sum1 << std::endl;
    std::cout << sum2 << std::endl;

}
//----------------------------------------------------------------- END INDIVIDUAL ALGORITHM IMPLEMENTATION



void calculateAndPrintBestRouteForCluster(int cluster)
{
    int filteredAdj[originalCountOfRequests * 2 + 1][originalCountOfRequests * 2 + 1];
    vector<Request> filteredRequests;
    validateAndBuildRequestCluster(cluster, filteredRequests);
    countOfRequests = filteredRequests.size();
    if (countOfRequests == 0)
        return; //TODO: Setup Zero Cluster manually
    createFilteredCostMatrixByClusteredRequests(filteredAdj);
   
    TSP(filteredAdj);
    checkCalculatedCluster();
    currentCluster.clusterDay = cluster;
    clusters[clusterCounter] = currentCluster;
    clusterCounter++;
    printResults(cluster);
}

int main() {
    setupMatchingTable();
    parseCosts("plop.csv");
    parseRequests("requests.csv");
    // vector<Node> nodes[N];F
    int clusterCount = 1;
    for (int i = 0; i < originalCountOfRequests; i++)
    {
        if (requests[i].releaseTime + 1 > clusterCount)
            clusterCount = requests[i].releaseTime + 1;
    }

    std::cout << "INITIAL CLUSTERING STARTED!" << std::endl;
    for (int i = 0; i < clusterCount; i++)
    {
        setupMatchingTable();
        resetData();
        calculateAndPrintBestRouteForCluster(i);
    }
    std::cout << "INITIAL CLUSTERING DONE!" << std::endl;

    doOptimizationSteps();
}








