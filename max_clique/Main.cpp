#include <istream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <set>
#include <random>
#include <tuple>
#include <iostream>
#include <stdio.h>
#include <time.h>
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
	setGraph("p_hat1500-3.clq");
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

bool isClique(set<int> clique, vector<vector<bool>> adj) {
	vector<int> cliqueList(clique.begin(), clique.end());
	for (int i = 0; i < cliqueList.size(); i++) {
		for (int j = 0; j < cliqueList.size(); j++) {
			if (i != j) {
				if (!adj[cliqueList[i]][cliqueList[j]]) {
					return false;
				}
			}
		}
	}
	return true;
}

vector<vector<bool>> fromInputFile(string filename) {
	ifstream inputFile;
	inputFile.open(filename);
	string line;
	int numVertices;
	vector <vector<bool>>adjacencyMatrix;
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
						adjacencyMatrix.resize(numVertices + 1, vector<bool>(numVertices + 1));
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
				adjacencyMatrix[i][j] = true;
				adjacencyMatrix[j][i] = true;
			}
		}
	}
	inputFile.close();
	return adjacencyMatrix;
}

set<int> getInitialSolution(vector<vector<bool>> adj) {
	random_device rd;
	mt19937 mersenneTwister(rd());
	uniform_int_distribution<int> dist(1, adj.size() - 1);
	int randomVertex = dist(mersenneTwister);
	vector<int> solutionList;
	set<int> solution;
	solutionList.push_back(randomVertex);
	solution.insert(randomVertex);
	bool foundThisIter = true;
	while (foundThisIter) {
		foundThisIter = false;
		for (int i = 1; i < adj.size(); i++) {
			if (solution.count(i) == 0) {
				bool add = true;
				for (int j = 0; j < solutionList.size(); j++) {
					add = add && (adj[i][solutionList[j]]); //is connected to all in list
				}
				if (add) {
					solutionList.push_back(i);
					solution.insert(i);
					foundThisIter = true;
				}
			}
		}
	}
	return solution;
}

tuple<vector<int>,vector<tuple<int,int>>> getAddSwapLists(set<int> sol, vector<vector<bool>> graph, set<int> tabu_set) {
	vector<int> addList;
	vector<tuple<int,int>> swapList;
	for (int i = 1; i < graph.size(); i++) {
		if (sol.count(i) == 0 && tabu_set.count(i) == 0) {
			int connected = 0;
			int disconnected = 0;
			int swapOut = -1;
			for (set<int>::iterator it = sol.begin(); it != sol.end(); ++it) {
				if (graph[i][*it]) {
					connected++;
				}
				else {
					disconnected++;
					swapOut = *it;
					if (disconnected > 1) {
						break;
					}
				}
			}
			if (connected == sol.size()) {
				addList.push_back(i);
			}
			else if (connected == sol.size() - 1) {
				swapList.push_back(tuple<int, int>(i, swapOut));
			}
		}
	}
	return tuple<vector<int>, vector<tuple<int,int>>>(addList, swapList);
}

vector<tuple<int, int>> getSwapList(set<int> sol, vector<vector<bool>> graph, set<int> tabu_set) {
	vector<tuple<int, int>> swapList;
	for (int swapIn = 1; swapIn < graph.size(); swapIn++) {
		if (sol.count(swapIn) == 0 && tabu_set.count(swapIn) == 0) {
			int connected = 0;
			int notConnected = 0;
			int swapOut = -1;
			for (set<int>::iterator it = sol.begin(); it != sol.end(); it++) {
				if (graph[swapIn][*it]) {
					connected++;
				}
				else {
					notConnected++;
					if (notConnected > 1) {
						break;
					}
					swapOut = *it;
				}
			}
			if (connected == sol.size() - 1) {
				swapList.push_back(tuple<int, int>(swapIn, swapOut));
			}
		}
	}
	return swapList;
}

void updateTabu(set<int> *tabu_set, map<int, int> *tabu_map) {
	vector<int> tabu(tabu_set->begin(), tabu_set->end());
	for (int i = 0; i < tabu.size(); i++) {
		(&tabu_map)[tabu[i]] = (&tabu_map)[tabu[i]] - 1;
		if ((&tabu_map)[tabu[i]] <= 0) {
			tabu_set->erase(tabu[i]);
			tabu_map->erase(tabu[i]);
		}
	}
}

string printClique(set<int> currBest, set<int> clique, vector<vector<bool>> graph, long long unimprovedCtr, long long iterationCtr) {
	return "size: " + to_string(clique.size())
		+ ", currBest_size: " + to_string(currBest.size())
		+ ", unimproved: " + to_string(unimprovedCtr)
		+ ", iteration: " + to_string(iterationCtr);
}

set<int> approxMaxClique(vector<vector<bool>> graph, long long unimprovedMax, long long maxIterations) {
	long long iterationCtr = 0;
	set<int> currMaxClique;
	random_device rd;
	mt19937 mersenneTwister(rd());
	while (iterationCtr < maxIterations) {
		set<int> s = getInitialSolution(graph);
		set<int> tabu_set;
		map<int, int> tabu_map;
		long long unimprovedCtr = 0;
		set<int> localBest = s;
		while (unimprovedCtr < unimprovedMax) {
			tuple<vector<int>, vector<tuple<int,int>>> addSwap = getAddSwapLists(s, graph, tabu_set);
			vector<int> add = get<0>(addSwap);
			if (add.size() > 0) {
				uniform_int_distribution<int> addDist(0, add.size() - 1);
				int randomAdd = addDist(mersenneTwister);
				s.insert(add[randomAdd]);
			}
			else {
				vector<tuple<int, int>> swap = get<1>(addSwap);
				if (swap.size() > 0) {
					uniform_int_distribution<int> swapDist(0, swap.size() - 1);
					int randomSwap = swapDist(mersenneTwister);
					int swapIn = get<0>(swap[randomSwap]);
					int swapOut = get<1>(swap[randomSwap]);
					s.insert(swapIn);
					s.erase(swapOut);
					tabu_set.insert(swapOut);
					uniform_int_distribution<int> tabuTimer(1, swap.size());
					tabu_map[swapOut] = tabuTimer(mersenneTwister) + 7;
				}
				else {
					uniform_int_distribution<int> deleteDist(0, s.size() - 1);
					int randomDelete = deleteDist(mersenneTwister);
					vector<int> sList(s.begin(), s.end());
					int deleteVertex = sList[randomDelete];
					s.erase(deleteVertex);
					tabu_set.insert(deleteVertex);
					tabu_map[deleteVertex] = 7;
				}
			}
			unimprovedCtr++;
			iterationCtr++;
			updateTabu(&tabu_set, &tabu_map);
			if (s.size() > localBest.size()) {
				string outString;
				outString += "Unimproved: " + to_string(unimprovedCtr - 1) + "\n";
				unimprovedCtr = 0;
				localBest = s;
				outString += "Found new local best: " + printClique(currMaxClique, localBest, graph, unimprovedCtr, iterationCtr) + "\n";
				cout << outString;
			}
		}
		if (localBest.size() > currMaxClique.size()) {
			currMaxClique = localBest;
			string outString;
			outString += "Found new currMax: " + printClique(currMaxClique, currMaxClique, graph, unimprovedCtr, iterationCtr) + "\n";
			outString += " isClique: " + (string)(isClique(currMaxClique, graph) ? "true" : "false") + "\n";
			vector<int> currMaxList(currMaxClique.begin(), currMaxClique.end());
			outString += "Best Clique So Far: [";
			for (int i : currMaxList) {
				outString += to_string(i) + " ";
			}
			outString += "]\n";
			cout << outString;
		}
	}
	return currMaxClique;
}

int main2() {
	cout << "start " << endl;
	vector<vector<bool>> adjacencyMatrix = fromInputFile("MANN_a81.clq");
	cout << "input read " << endl;
	set<int> maxClique = approxMaxClique(adjacencyMatrix, 4000, 1000000);
	cout << "maxClique: " << maxClique.size() << " isClique: " << (isClique(maxClique, adjacencyMatrix) ? "true" : "false") << endl;
	char l;
	cin >> l;
	return 0;
}