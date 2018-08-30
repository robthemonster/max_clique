#include <fstream>
#include <sstream>
#include <random>
#include <iostream>
#include <chrono>

using namespace std;

bool **graph;
int * localBest;
int localBestSize = 0;
int * bestSolution;
int bestSolutionSize = 0;
int *solution;
int solutionSize = 0;
bool * solutionContains;
int * addList;
int addListSize = 0;
int * swapList;
int * swapOutList;
int * swapOutIndexList;
int swapListSize = 0;
int * tabuList;
int * tabuMap;
bool * tabuContains;
int tabuListSize = 0;
int numVertices;

random_device rd;
mt19937 mersenneTwister(rd());

void setGraph(string);
void runHeuristic(long long, long long);
void cleanGraph();
void setInitialSolution();
void resetLists();
void setAddAndSwapLists();
void addRandom();
void swapRandom();
void removeRandom();
void decrementTabu();
string isBestAClique();

int main() {
	setGraph("p_hat500-1.clq");
	runHeuristic(4000, 100000000);
	cout << "Best solution found: " << bestSolutionSize << " isClique: " << isBestAClique() << endl;
	cout << "[";
	for (int i = 0; i < bestSolutionSize; i++) {
		cout << bestSolution[i] + " ";
	}
	cout << "]" << endl;
	cleanGraph();
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
	auto startTime = chrono::high_resolution_clock::now();
	long long iterationCtr = 0;
	while (iterationCtr < maxIterations) {
		setInitialSolution();
		resetLists();
		long long unimprovedCtr = 0;
		while (unimprovedCtr < maxUnimproved) {
			setAddAndSwapLists();
			if (addListSize > 0) {
				addRandom();
			}
			else {
				if (swapListSize > 0) {
					swapRandom();
				}
				else if (solutionSize > 0) {
					removeRandom();
				}
			}
			unimprovedCtr++;
			iterationCtr++;
			decrementTabu();
			if (solutionSize > localBestSize) {
				for (int i = 0; i < solutionSize; i++) {
					localBest[i] = solution[i];
				}
				localBestSize = solutionSize;
				unimprovedCtr = 0;
			}
		}
		if (localBestSize > bestSolutionSize) {
			for (int i = 0; i < localBestSize; i++) {
				bestSolution[i] = localBest[i];
			}
			bestSolutionSize = localBestSize;
			auto diff = chrono::duration_cast<chrono::milliseconds>(chrono::high_resolution_clock::now() - startTime).count();
			cout << "Iteration:" << iterationCtr << " Clique Size:" << bestSolutionSize << " Timer:" << diff << "ms" << endl;
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

void addRandom() {
	uniform_int_distribution<int> dist(0, addListSize - 1);
	int randomAddIndex = dist(mersenneTwister);
	solution[solutionSize] = addList[randomAddIndex];
	solutionSize++;
	solutionContains[addList[randomAddIndex]] = true;
}

void swapRandom() {
	uniform_int_distribution<int> dist(0, swapListSize - 1);
	int randomSwapIndex = dist(mersenneTwister);
	solution[swapOutIndexList[randomSwapIndex]] = swapList[randomSwapIndex];
	uniform_int_distribution<int> tabuTimer(1, swapListSize);
	int timer = tabuTimer(mersenneTwister);
	tabuList[tabuListSize] = timer + 7;
	tabuMap[tabuListSize] = swapOutList[randomSwapIndex];
	tabuListSize++;
	tabuContains[swapOutList[randomSwapIndex]] = true;
	solutionContains[swapList[randomSwapIndex]] = true;
	solutionContains[swapOutList[randomSwapIndex]] = false;
	return;
}

void removeRandom() {
	uniform_int_distribution<int> dist(0, solutionSize - 1);
	int randomRemoveIndex = dist(mersenneTwister);
	tabuList[tabuListSize] = 7;
	tabuMap[tabuListSize] = solution[randomRemoveIndex];
	tabuListSize++;
	tabuContains[solution[randomRemoveIndex]] = true;
	solutionContains[solution[randomRemoveIndex]] = false;
	swap(solution[randomRemoveIndex], solution[solutionSize - 1]);
	solutionSize--;
}

void setAddAndSwapLists() {
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
			}
			else if (connected == solutionSize - 1) {
				swapList[swapListSize] = i;
				swapOutList[swapListSize] = swapOut;
				swapOutIndexList[swapListSize] = swapOutIndex;
				swapListSize++;
			} 
		}
	}
}

void resetLists() {
	addListSize = 0;
	swapListSize = 0;
}

void setInitialSolution() {
	fill(tabuContains, tabuContains + numVertices + 1, false);
	fill(solutionContains, solutionContains + numVertices + 1, false);
	uniform_int_distribution<int> dist(1, numVertices);
	int randomVertex = dist(mersenneTwister);
	solution[0] = randomVertex;
	solutionSize = 1;
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
				foundMore = true;
			}
		}
	} while (foundMore);
}

void setGraph(string filename) {
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
						for (int i = 0; i <= numVertices; i++) {
							graph[i] = new bool[numVertices + 1]{ false };
						}
						solution = new int[numVertices];
						solutionContains = new bool[numVertices + 1];
						addList = new int[numVertices];
						swapList = new int[numVertices];
						swapOutList = new int[numVertices];
						swapOutIndexList = new int[numVertices];
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
}

void cleanGraph() {
	for (int i = 0; i < numVertices + 1; i++) {
		delete graph[i];
	}
	delete graph;
	delete solution;
	delete solutionContains;
	delete localBest;
	delete bestSolution;
	delete addList;
	delete swapList;
	delete swapOutList;
	delete swapOutIndexList;
	delete tabuList;
	delete tabuMap;
	delete tabuContains;
}