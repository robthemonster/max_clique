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

using namespace std;

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

vector<int> getAddList(set<int> sol, vector<vector<bool>> graph, set<int> tabu_set) {
	vector<int> addList;
	for (int i = 1; i < graph.size(); i++) {
		if (sol.count(i) == 0 && tabu_set.count(i) == 0) {
			bool connected = true;
			for (set<int>::iterator it = sol.begin(); it != sol.end(); it++) {
				connected = connected && graph[i][*it];
				if (!connected) {
					break;
				}
			}
			if (connected) {
				addList.push_back(i);
			}
		}
	}
	return addList;
}

vector<tuple<int, int>> getSwapList(set<int> sol, vector<vector<bool>> graph, set<int> tabu_set) {
	vector<tuple<int, int>> swapList;
	for (int swapIn = 1; swapIn < graph.size(); swapIn++) {
		if (sol.count(swapIn) == 0 && tabu_set.count(swapIn) == 0) {
			int connected = 0;
			int swapOut = -1;
			for (set<int>::iterator it = sol.begin(); it != sol.end(); it++) {
				if (graph[swapIn][*it]) {
					connected++;
				}
				else {
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
		tabu_map->at(tabu[i]) = tabu_map->at(tabu[i]) - 1;
		if (tabu_map->at(tabu[i]) <= 0) {
			tabu_set->erase(tabu[i]);
			tabu_map->erase(tabu[i]);
		}
	}
}

string printClique(set<int> currBest, set<int> clique, vector<vector<bool>> graph, long long unimprovedCtr, long long iterationCtr) {
	return "size: " + to_string(clique.size())
		+ ", isClique: " + (isClique(clique, graph) ? "true" : "false")
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
			vector<int> add = getAddList(s, graph, tabu_set);
			if (add.size() > 0) {
				uniform_int_distribution<int> addDist(0, add.size() - 1);
				int randomAdd = addDist(mersenneTwister);
				s.insert(add[randomAdd]);
			}
			else {
				vector<tuple<int, int>> swap = getSwapList(s, graph, tabu_set);
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
				cout << "Unimproved ctr: " << unimprovedCtr << endl;
				unimprovedCtr = 0;
				localBest = s;
				cout << "Found new local best: " << printClique(currMaxClique, localBest, graph, unimprovedCtr, iterationCtr) << endl;
			}
		}
		if (localBest.size() > currMaxClique.size()) {
			currMaxClique = localBest;
			cout << "Found new currMax: " << printClique(currMaxClique, currMaxClique, graph, unimprovedCtr, iterationCtr) << endl;
			vector<int> currMaxList(currMaxClique.begin(), currMaxClique.end());
			cout << "Best Clique So Far: [";
			for (int i : currMaxList) {
				cout << i << " ";
			}
			cout << "]" << endl;
		}
	}
	return currMaxClique;
}

int main() {
	cout << "start " << endl;
	vector<vector<bool>> adjacencyMatrix = fromInputFile("p_hat1500-3.clq");
	cout << "input read " << endl;
	set<int> maxClique = approxMaxClique(adjacencyMatrix, 100, 1000000);
	cout << "maxClique: " << maxClique.size() << " isClique: " << (isClique(maxClique, adjacencyMatrix) ? "true" : "false") << endl;
	char l;
	cin >> l;
	return 0;
}