#include <fstream>
#include <sstream>
#include <random>
#include <iostream>
#include <chrono>

using namespace std;

bool **graph; //2d adj matrix
int **notConnected; //adj list
int * indexMap;
int * notConnectedLength;
int * numInSolutionNotConnectedTo;
int * weights; //weighted of nodes
int * localBest; //best solution in local search space
int localBestSize = 0;
long localBestWeight = 0;
int * bestSolution; //best solution so far
int bestSolutionSize = 0;
long bestSolutionWeight = 0;
int *solution; //current working solution
int solutionSize = 0;
long solutionWeight = 0;
bool * solutionContains; //map indicating whether a given vertex is contained in solution
int * addList; //list of vertices to add to solution
int addListSize = 0;
int * swapList; // list of vertices to swap into solution
int * swapBuddy;
int swapListSize = 0;
int greatestDelta = INT16_MIN;
int * tiedDeltas;
int  * tiedDeltasNeighborhoods; //tiedDeltasNeighborhoods[i] \in {0,1,2} correspond to add/swap/delete
int tiedDeltasSize = 0; 
int * tabuList; //list of tabu vertices
int * tabuMap; //corresponding map to the vertex number of a given tabu list entry
bool * tabuContains; //map indicating whether a given vertex is contained in the tabu list
int tabuListSize = 0;
int numVertices;
chrono::high_resolution_clock::time_point startTime;
long long iterationCtr;
long long iterationTotal;
long long iteration[100];
int currIt;
int bestKnown;

const int ADD = 0, SWAP = 1, REMOVE = 2;

random_device rd;
mt19937 mersenneTwister(rd());

void parseGraphFromFile(string);
void runHeuristic(long long, long long);
void deallocateGraph();
void setInitialSolution();
void addToSolution(int);
void swapIntoSolution(int);
void removeFromSolution(int);
void removeFromAdd(int);
void removeFromSwap(int);
void addToAdd(int);
void addToSwap(int, int);
void incrementNumInSolutionNotConnectedTo(int);
void decrementNumInSolutionNotConnectedTo(int);
bool selectBestDelta();
void decrementTabu();
string isBestAClique();

int main(int argc, char **argv) {
	char * fileName = argv[1];
	bestKnown = stoi(argv[2]);
	parseGraphFromFile(fileName);
	long long weightTotal = 0;
	iterationTotal = 0;
	for (currIt = 0; currIt < 100; currIt++) {
		runHeuristic(4000, (100000000 / 4000) + 1);
		iterationTotal += iterationCtr;
		cout << currIt + 1 << ") Best solution found: " << bestSolutionSize << " isClique: " << isBestAClique() << " Iteration: " << iteration[currIt] << endl;
		long weight = 0;
		cout << "[";
		for (int j = 0; j < bestSolutionSize; j++) {
			cout << bestSolution[j] << " ";
			weight += weights[bestSolution[j]];
		}
		cout << "]" << endl;
		cout << "Weight: " << weight << endl;
		weightTotal += weight;
		localBestSize = 0;
		localBestWeight = 0;
		bestSolutionSize = 0;
		bestSolutionWeight = 0;
	}
	cout << "Average weight: " << (weightTotal / 100.0) << " Average time: " << chrono::duration_cast<chrono::milliseconds>(chrono::high_resolution_clock::now() - startTime).count() / 100 << "ms" << endl;
	deallocateGraph();
	char c;
	cin >> c;
}

string isBestAClique() {
	for (int i = 0; i < bestSolutionSize; i++) {
		for (int j = 0; j < bestSolutionSize; j++) {
			if (i != j) {
				if (!graph[bestSolution[i]][bestSolution[j]]) {
					return "false";
				}
			}
		}
	}
	return "true";
}

void runHeuristic(long long maxUnimproved, long long maxIterations) {
	iterationCtr = 0;
	startTime = chrono::high_resolution_clock::now();
	iterationTotal = 0;
	while (iterationCtr < maxIterations) {
		setInitialSolution();
		long long unimprovedCtr = 0;
		while (unimprovedCtr < maxUnimproved) {
			if (selectBestDelta()) {
				unimprovedCtr++;
				decrementTabu();
				if (solutionWeight > localBestWeight) {
					for (int i = 0; i < solutionSize; i++) {
						localBest[i] = solution[i];
					}
					localBestSize = solutionSize;
					localBestWeight = solutionWeight;
					if (localBestWeight == bestKnown) {
						break;
					}
					//unimprovedCtr = 0;
				}
			}
			else {
				break;
			}
		}
		iterationCtr++;
		if (localBestWeight > bestSolutionWeight) {
			for (int i = 0; i < localBestSize; i++) {
				bestSolution[i] = localBest[i];
			}
			bestSolutionSize = localBestSize;
			bestSolutionWeight = localBestWeight;
			auto diff = chrono::duration_cast<chrono::milliseconds>(chrono::high_resolution_clock::now() - startTime).count();
			iterationTotal += unimprovedCtr;
			iteration[currIt] = iterationTotal;
			cout << "Iteration:" << iteration[currIt] << " Clique Size:" << bestSolutionSize << " Clique Weight: " << bestSolutionWeight << " Timer:" << diff << "ms" << endl;
			if (bestSolutionWeight == bestKnown) {
				return;
			}
		}
	}
}

void decrementTabu() {
	for (int i = 0; i < tabuListSize; i++) {
		tabuList[i]--;
		if (tabuList[i] <= 0) {
			tabuContains[tabuMap[i]] = false;
			swap(tabuList[i], tabuList[tabuListSize - 1]);
			swap(tabuMap[i], tabuMap[tabuListSize - 1]);
			tabuListSize--;
		}
	}
}

bool selectBestDelta()
{
	int greatestDelta = INT16_MIN;
	for (int i = 0; i < addListSize; i++) {
		bool tabu = tabuContains[addList[i]];
		if (weights[addList[i]] > greatestDelta && !tabu) {
			greatestDelta = weights[addList[i]];
			tiedDeltasSize = 0;
			tiedDeltas[tiedDeltasSize] = i;
			tiedDeltasNeighborhoods[tiedDeltasSize] = ADD;
			tiedDeltasSize++;
		}
		else if (weights[addList[i]] == greatestDelta && !tabu) {
			tiedDeltas[tiedDeltasSize] = i;
			tiedDeltasNeighborhoods[tiedDeltasSize] = ADD;
			tiedDeltasSize++;
		}
	}

	for (int i = 0; i < swapListSize; i++) {
		bool tabu = tabuContains[swapList[i]];
		int swapDelta = weights[swapList[i]] - weights[swapBuddy[swapList[i]]];
		if (swapDelta > greatestDelta && !tabu) {
			greatestDelta = swapDelta;
			tiedDeltasSize = 0;
			tiedDeltas[tiedDeltasSize] = i;
			tiedDeltasNeighborhoods[tiedDeltasSize] = SWAP;
			tiedDeltasSize++;
		}
		else if (swapDelta == greatestDelta && tabu) {
			tiedDeltas[tiedDeltasSize] = i;
			tiedDeltasNeighborhoods[tiedDeltasSize] = SWAP;
			tiedDeltasSize++;
		}
	}
	if (tiedDeltasSize == 0) {
		for (int i = 0; i < solutionSize; i++) {
			if (weights[solution[i]] * -1 > greatestDelta) {
				greatestDelta = -1 * weights[solution[i]];
				tiedDeltasSize = 0;
				tiedDeltas[tiedDeltasSize] = i;
				tiedDeltasNeighborhoods[tiedDeltasSize] = REMOVE;
				tiedDeltasSize++;
			}
			else if (weights[solution[i]] * -1 == greatestDelta) {
				tiedDeltas[tiedDeltasSize] = i;
				tiedDeltasNeighborhoods[tiedDeltasSize] = REMOVE;
				tiedDeltasSize++;
			}
		}
	}
	if (tiedDeltasSize > 0) {
		uniform_int_distribution<int> dist(0, tiedDeltasSize - 1);
		int randomDeltaIndex = dist(mersenneTwister);
		if (tiedDeltasNeighborhoods[randomDeltaIndex] == ADD) {
			int vertex = addList[tiedDeltas[randomDeltaIndex]];
			addToSolution(vertex);
		}
		else if (tiedDeltasNeighborhoods[randomDeltaIndex] == SWAP) {
			int vertex = swapList[tiedDeltas[randomDeltaIndex]];
			int swapBud = swapBuddy[vertex];
			swapIntoSolution(vertex);
			uniform_int_distribution<int> tabuTimer(1, swapListSize);
			int timer = tabuTimer(mersenneTwister);
			tabuList[tabuListSize] = timer + 7;
			tabuMap[tabuListSize] = swapBud;
			tabuListSize++;
			tabuContains[swapBud] = true;
		}
		else {
			int vertex = solution[tiedDeltas[randomDeltaIndex]];
			for (int i = 0; i < notConnectedLength[vertex]; i++) {
				int disconnectedNode = notConnected[vertex][i];
				int disconnectedNum = numInSolutionNotConnectedTo[disconnectedNode];
				if (disconnectedNum == 2) {
					if (swapBuddy[disconnectedNode] == vertex) {
						tiedDeltasSize = 0;
						greatestDelta = INT16_MIN;
						return false;
					}
				}
			}
			removeFromSolution(tiedDeltas[randomDeltaIndex]);
			tabuList[tabuListSize] = 7;
			tabuMap[tabuListSize] = vertex;
			tabuListSize++;
			tabuContains[vertex] = true;
		}
	} else {
		return false;
	}
	tiedDeltasSize = 0;
	greatestDelta = INT16_MIN;
	return true;
}

void setInitialSolution() {
	swapListSize = 0;
	solutionSize = 0;
	solutionWeight = 0;
	addListSize = 0;
	fill(tabuContains, tabuContains + numVertices + 1, false);
	fill(solutionContains, solutionContains + numVertices + 1, false);
	for (int i = 1; i <= numVertices; i++) {
		numInSolutionNotConnectedTo[i] = 0;
		solutionContains[i] = false;
		addList[i - 1] = i;
		indexMap[i] = i - 1;
	}
	addListSize = numVertices;
	uniform_int_distribution<int> dist(1, numVertices);
	int randomVertex = dist(mersenneTwister);
	addToSolution(randomVertex);
	while (addListSize > 0) {
			addToSolution(addList[addListSize - 1]);
	}
}

void incrementNumInSolutionNotConnectedTo(int vertexInSolution)
{
	for (int i = 0; i < notConnectedLength[vertexInSolution]; i++) {
		int vertex = notConnected[vertexInSolution][i];
		numInSolutionNotConnectedTo[vertex]++;
		int numDisconnected = numInSolutionNotConnectedTo[vertex];
		if (numDisconnected == 1 && !solutionContains[vertex]) {
			removeFromAdd(vertex);
			addToSwap(vertex, vertexInSolution);
		}
		else if (numDisconnected == 2) {
			removeFromSwap(vertex);
		}
	}
}

void decrementNumInSolutionNotConnectedTo(int leavingVertex)
{
	for (int i = 0; i < notConnectedLength[leavingVertex]; i++) {
		int vertex = notConnected[leavingVertex][i];
		numInSolutionNotConnectedTo[vertex]--;
		int numDisconnected = numInSolutionNotConnectedTo[vertex];
		if (numDisconnected == 1) {
			addToSwap(vertex, swapBuddy[vertex]);
		} else if (numDisconnected == 0 && !solutionContains[vertex]) {
			removeFromSwap(vertex);
			addToAdd(vertex);
		}
	}
}

void addToSolution(int vertex)
{
	removeFromAdd(vertex);
	solution[solutionSize] = vertex;
	solutionSize++;
	solutionWeight += weights[vertex];
	solutionContains[vertex] = true;
	indexMap[vertex] = solutionSize - 1;
	incrementNumInSolutionNotConnectedTo(vertex);
}

void swapIntoSolution(int vertexIn) {
	int vertexOut, indexOut;
	for (int i = 0; i < solutionSize; i++) {
		vertexOut = solution[i];
		indexOut = i;
		if (!graph[vertexIn][vertexOut]) {
			break;
		}
	}
	int indexIn = indexMap[vertexIn];
	solutionWeight += weights[vertexIn] - weights[vertexOut];
	solutionContains[vertexIn] = true;
	solution[solutionSize] = vertexIn;


	removeFromSwap(vertexIn);
	indexMap[vertexIn] = solutionSize;
	solutionSize++;

	incrementNumInSolutionNotConnectedTo(vertexIn);

	solutionContains[vertexOut] = false;
	solution[indexOut] = solution[solutionSize - 1];
	indexMap[solution[indexOut]] = indexOut;
	solutionSize--;
	addToSwap(vertexOut, vertexIn);
	
	decrementNumInSolutionNotConnectedTo(vertexOut);

}

void removeFromSolution(int index) {
	int vertex = solution[index];
	solution[index] = solution[solutionSize - 1];
	indexMap[solution[index]] = index;
	solutionSize--;
	solutionWeight -= weights[vertex];
	solutionContains[vertex] = false;
	decrementNumInSolutionNotConnectedTo(vertex);
	addToAdd(vertex);
}

void removeFromAdd(int vertexToRemove)
{
		int index = indexMap[vertexToRemove];
		addList[index] = addList[addListSize - 1];
		addListSize--;
		if (addListSize > 0) {
			indexMap[addList[index]] = index;
		}
}

void removeFromSwap(int vertexToRemove)
{
	int index = indexMap[vertexToRemove];
	swapList[index] = swapList[swapListSize - 1];
	swapListSize--;
	if (swapListSize > 0) {
		indexMap[swapList[index]] = index;
	}
}

void addToAdd(int vertex)
{
	addList[addListSize] = vertex;
	indexMap[vertex] = addListSize;
	addListSize++;
}

void addToSwap(int swapIn, int swapOut)
{
	swapList[swapListSize] = swapIn;
	swapBuddy[swapIn] = swapOut;
	indexMap[swapIn] = swapListSize;
	swapListSize++;
}

void parseGraphFromFile(string filename) {
	ifstream inputFile;
	inputFile.open(filename);
	string line;
	while (getline(inputFile, line)) {
		if (line != "") {
			if (line[0] == 'p') {
				stringstream stringStream(line);
				string curr;
				int ctr = 0;
				while (stringStream >> curr) {
					ctr++;
					if (ctr == 3) {
						numVertices = stoi(curr);
						graph = new bool*[numVertices + 1];
						indexMap = new int[numVertices + 1];
						notConnected = new int*[numVertices + 1];
						notConnectedLength = new int[numVertices + 1]{ 0 };
						numInSolutionNotConnectedTo = new int[numVertices + 1]{ 0 };
						weights = new int[numVertices + 1];
						for (int i = 0; i <= numVertices; i++) {
							weights[i] = (i % 200) + 1;
							graph[i] = new bool[numVertices + 1]{ false };
							notConnected[i] = new int[numVertices + 1]{ -1 };
						}
						solution = new int[numVertices];
						solutionContains = new bool[numVertices + 1];
						addList = new int[numVertices];
						swapList = new int[numVertices];
						swapBuddy = new int[numVertices + 1];
						tiedDeltas = new int[numVertices];
						tiedDeltasNeighborhoods = new int[numVertices];
						tabuList = new int[numVertices];
						tabuMap = new int[numVertices];
						tabuContains = new bool[numVertices + 1];
						bestSolution = new int[numVertices];
						localBest = new int[numVertices];
					}
				}
			}
			else if (line[0] == 'e') {
				stringstream stringStream(line);
				string curr;
				int ctr = 0;
				int i, j;
				while (stringStream >> curr) {
					ctr++;
					if (ctr == 2) {
						i = stoi(curr);
					}
					else if (ctr == 3) {
						j = stoi(curr);
						break;
					}
				}
				graph[i][j] = true;
				graph[j][i] = true;
			}
		}
	}
	inputFile.close();
	for (int i = 1; i <= numVertices; i++) {
		for (int j = 1; j <= numVertices; j++) {
			if (i != j && !graph[i][j]) {
				notConnected[i][notConnectedLength[i]] = j;
				notConnectedLength[i]++;
			}
		}
	}
}

void deallocateGraph() {
	for (int i = 0; i < numVertices + 1; i++) {
		delete[] graph[i];
		delete[] notConnected[i];
	}
	delete[] numInSolutionNotConnectedTo;
	delete[] indexMap;
	delete[] graph;
	delete[] weights;
	delete[] solution;
	delete[] solutionContains;
	delete[] localBest;
	delete[] bestSolution;
	delete[] addList;
	delete[] swapList;
	delete[] tiedDeltas;
	delete[] tiedDeltasNeighborhoods;
	delete[] tabuList;
	delete[] tabuMap;
	delete[] tabuContains;
}