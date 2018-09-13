#include <fstream>
#include <sstream>
#include <random>
#include <iostream>
#include <chrono>

using namespace std;

bool **graph; //2d adj matrix
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
int * swapOutList; //correspecitng list of vertices to swap out
int * swapOutIndexList; //index of the element to be swapped out of solution
int swapListSize = 0;
int greatestDelta = INT8_MIN;
int * tiedDeltas;
int  * tiedDeltasNeighborhoods; //tiedDeltasNeighborhoods[i] \in {0,1,2} correspond to add/swap/delete
int tiedDeltasSize = 0; 
int * tabuList; //list of tabu vertices
int * tabuMap; //corresponding map to the vertex number of a given tabu list entry
bool * tabuContains; //map indicating whether a given vertex is contained in the tabu list
int tabuListSize = 0;
int numVertices;

const int ADD = 0, SWAP = 1, REMOVE = 2;

random_device rd;
mt19937 mersenneTwister(rd());

void parseGraphFromFile(string);
void runHeuristic(long long, long long);
void deallocateGraph();
void setInitialSolution();
void findNeighborhood();
void selectBestDelta();
void decrementTabu();
string isBestAClique();

int main() {
	parseGraphFromFile("MANN_a81.clq");
	runHeuristic(4000, 100000000);
	cout << "Best solution found: " << bestSolutionSize << " isClique: " << isBestAClique() << endl;
	long weight = 0;
	cout << "[";
	for (int i = 0; i < bestSolutionSize; i++) {
		cout << bestSolution[i] << " ";
		weight += weights[bestSolution[i]];
	}
	cout << "]" << endl;
	cout << "Weight: " << weight << endl;
	
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
	chrono::high_resolution_clock::time_point startTime = chrono::high_resolution_clock::now();
	long long iterationCtr = 0;
	while (iterationCtr < maxIterations) {
		setInitialSolution();
		long long unimprovedCtr = 0;
		while (unimprovedCtr < maxUnimproved) {
			findNeighborhood();
			selectBestDelta();
			unimprovedCtr++;
			iterationCtr++;
			decrementTabu();
			if (solutionWeight > localBestWeight) {
				for (int i = 0; i < solutionSize; i++) {
					localBest[i] = solution[i];
				}
				localBestSize = solutionSize;
				localBestWeight = solutionWeight;
				unimprovedCtr = 0;
			}
		}
		if (localBestWeight > bestSolutionWeight) {
			for (int i = 0; i < localBestSize; i++) {
				bestSolution[i] = localBest[i];
			}
			bestSolutionSize = localBestSize;
			bestSolutionWeight = localBestWeight;
			auto diff = chrono::duration_cast<chrono::milliseconds>(chrono::high_resolution_clock::now() - startTime).count();
			cout << "Iteration:" << iterationCtr << " Clique Size:" << bestSolutionSize << " Clique Weight: " << bestSolutionWeight << " Timer:" << diff << "ms" << endl;
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

void selectBestDelta()
{
	uniform_int_distribution<int> dist(0, tiedDeltasSize - 1);
	int randomDeltaIndex = dist(mersenneTwister);
	int listIndex = tiedDeltas[randomDeltaIndex];
	if (tiedDeltasNeighborhoods[randomDeltaIndex] == ADD) {
		solution[solutionSize] = addList[listIndex];
		solutionSize++;
		solutionWeight += weights[solution[solutionSize - 1]];
		solutionContains[solution[solutionSize - 1]] = true;
	}
	else if (tiedDeltasNeighborhoods[randomDeltaIndex] == SWAP) {
		solution[swapOutIndexList[listIndex]] = swapList[listIndex];
		uniform_int_distribution<int> tabuTimer(1, swapListSize);
		int timer = tabuTimer(mersenneTwister);
		tabuList[tabuListSize] = timer + 7;
		tabuMap[tabuListSize] = swapOutList[listIndex];
		tabuListSize++;
		tabuContains[swapOutList[listIndex]] = true;
		solutionContains[swapList[listIndex]] = true;
		solutionContains[swapOutList[listIndex]] = false;
		solutionWeight += weights[swapList[listIndex]] - weights[swapOutList[listIndex]];
	}
	else {
		tabuList[tabuListSize] = 7;
		tabuMap[tabuListSize] = solution[listIndex];
		tabuListSize++;
		tabuContains[solution[listIndex]] = true;
		solutionContains[solution[listIndex]] = false;
		solutionWeight -= weights[solution[listIndex]];
		swap(solution[listIndex], solution[solutionSize - 1]);
		solutionSize--;
	}
	tiedDeltasSize = 0;
	greatestDelta = INT8_MIN;
}

void findNeighborhood() {
	addListSize = 0, swapListSize = 0;
	for (int i = 1; i <= numVertices; i++) {
		int connected = 0;
		int disconnected = 0;
		int swapOut = -1;
		int swapOutIndex = -1;
		if (!tabuContains[i] && !solutionContains[i]) {
			for (int j = 0; j < solutionSize; j++) {
				if (graph[i][solution[j]]) {
					connected++;
				}
				else {
					swapOut = solution[j];
					swapOutIndex = j;
					disconnected++;
					if (disconnected > 1) {
						break;
					}
				}
			}
			if (connected == solutionSize) {
				addList[addListSize] = i;
				addListSize++;
				int addDelta = weights[i];
				if (addDelta > greatestDelta) {
					greatestDelta = addDelta;
					tiedDeltas[0] = addListSize - 1;
					tiedDeltasNeighborhoods[0] = ADD;
					tiedDeltasSize = 1;
				}
				else if (addDelta == greatestDelta) {
					tiedDeltas[tiedDeltasSize] = addListSize - 1;
					tiedDeltasNeighborhoods[tiedDeltasSize] = ADD;
					tiedDeltasSize++;
				}
			}
			else if (connected == solutionSize - 1) {
				swapList[swapListSize] = i;
				swapOutList[swapListSize] = swapOut;
				swapOutIndexList[swapListSize] = swapOutIndex;
				swapListSize++;
				int swapDelta = weights[i] - weights[swapOut];
				if (swapDelta > greatestDelta) {
					greatestDelta = swapDelta;
					tiedDeltas[0] = swapListSize - 1;
					tiedDeltasNeighborhoods[0] = SWAP;
					tiedDeltasSize = 1;
				}
				else if (swapDelta == greatestDelta) {
					tiedDeltas[tiedDeltasSize] = swapListSize - 1;
					tiedDeltasNeighborhoods[tiedDeltasSize] = SWAP;
					tiedDeltasSize++;
				}
			} 
		}
	}
	for (int i = 0; i < solutionSize; i++) {
		int removeDelta = weights[solution[i]] * -1;
		if (removeDelta > greatestDelta) {
			greatestDelta = solution[i] * -1;
			tiedDeltas[0] = i;
			tiedDeltasNeighborhoods[0] = REMOVE;
			tiedDeltasSize = 1;
		}
		else if (removeDelta == greatestDelta) {
			tiedDeltas[tiedDeltasSize] = i;
			tiedDeltasNeighborhoods[tiedDeltasSize] = REMOVE;
			tiedDeltasSize++;
		}
	}
}

void setInitialSolution() {
	fill(tabuContains, tabuContains + numVertices + 1, false);
	fill(solutionContains, solutionContains + numVertices + 1, false);
	uniform_int_distribution<int> dist(1, numVertices);
	int randomVertex = dist(mersenneTwister);
	solution[0] = randomVertex;
	solutionSize = 1;
	solutionWeight = weights[solution[0]];
	solutionContains[randomVertex] = true;
	bool foundMore = false;
	do {
		foundMore = false;
		for (int i = 1; i <= numVertices; i++) {
			bool connected = true;
			for (int j = 0; j < solutionSize; j++) {
				if (!graph[i][solution[j]]) {
					connected = false;
					break;
				}
			}
			if (connected) {
				solution[solutionSize] = i;
				solutionContains[i] = true;
				solutionSize++;
				solutionWeight += weights[i];
				foundMore = true;
			}
		}
	} while (foundMore);
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
						weights = new int[numVertices + 1];
						for (int i = 0; i <= numVertices; i++) {
							weights[i] = (i % 200) + 1;
							graph[i] = new bool[numVertices + 1]{ false };
						}
						solution = new int[numVertices];
						solutionContains = new bool[numVertices + 1];
						addList = new int[numVertices];
						swapList = new int[numVertices];
						swapOutList = new int[numVertices];
						swapOutIndexList = new int[numVertices];
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
}

void deallocateGraph() {
	for (int i = 0; i < numVertices + 1; i++) {
		delete[] graph[i];
	}
	delete[] graph;
	delete[] weights;
	delete[] solution;
	delete[] solutionContains;
	delete[] localBest;
	delete[] bestSolution;
	delete[] addList;
	delete[] swapList;
	delete[] swapOutList;
	delete[] swapOutIndexList;
	delete[] tiedDeltas;
	delete[] tiedDeltasNeighborhoods;
	delete[] tabuList;
	delete[] tabuMap;
	delete[] tabuContains;
}