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

set<int> getAddSet(set<int> sol, vector<vector<bool>> graph, set<int> tabu_set) {
	set<int> addSet;
	vector<int> solList(sol.begin(), sol.end());
	for (int i = 1; i < graph.size(); i++) {
		if (sol.count(i) == 0 && tabu_set.count(i) == 0) {
			bool connected = true;
			for (int j = 0; j < solList.size(); j++) {
				connected = connected && graph[i][solList[j]];
				if (!connected) {
					break;
				}
			}
			if (connected) {
				addSet.insert(i);
			}
		}
	}
	return addSet;
}

set<tuple<int,int>> getSwapSet(set<int> sol, vector<vector<bool>> graph, set<int> tabu_set) {
	set<tuple<int,int>> swapSet;
	for (int swapIn = 1; swapIn < graph.size(); swapIn++) {
		if (sol.count(swapIn) == 0 && tabu_set.count(swapIn) == 0) {
			int connected = 0;
			int swapOut = -1;
			vector<int> solList(sol.begin(), sol.end());
			for (int j = 0; j < solList.size(); j++) {
				if (graph[swapIn][solList[j]]) {
					connected++;
				}
				else {
					swapOut = solList[j];
				}
			}
			if (connected == sol.size() - 1) {
				swapSet.insert(tuple<int,int>(swapIn, swapOut));
			}
		}
	}
	return swapSet;
}

void updateTabu(set<int> *tabu_set, map<int, int> *tabu_list) {
	vector<int> tabu(tabu_set->begin(), tabu_set->end());
	for (int i = 0; i < tabu.size(); i++) {
		tabu_list->at(tabu[i]) = tabu_list->at(tabu[i]) - 1;
		if (tabu_list->at(tabu[i]) <= 0) {
			tabu_set->erase(tabu[i]);
		}
	}
}

set<int> approxMaxClique(vector<vector<bool>> graph, long long unimprovedMax, long long maxIterations) {
	long long iterationCtr = 0;
	set<int> currMaxClique;
	random_device rd;
	mt19937 mersenneTwister(rd());
	while (iterationCtr < maxIterations) {
		set<int> s = getInitialSolution(graph);
		set<int> tabu_set;
		map<int, int> tabu_list;
		long long unimprovedCtr = 0;
		set<int> localBest = s;
		while (unimprovedCtr < unimprovedMax) {
			set<int> add = getAddSet(s, graph, tabu_set);
			set<tuple<int,int>> swap = getSwapSet(s, graph, tabu_set);
			if (add.size() > 0) {
				uniform_int_distribution<int> addDist(0, add.size() - 1);
				int randomAdd = addDist(mersenneTwister);
				vector<int> addList(add.begin(), add.end());
				s.insert(addList[randomAdd]);
			}
			else if (swap.size() > 0) {
				uniform_int_distribution<int> swapDist(0, swap.size() - 1);
				int randomSwap = swapDist(mersenneTwister);
				vector<tuple<int, int>> swapList(swap.begin(), swap.end());
				int swapIn = get<0>(swapList[randomSwap]);
				int swapOut = get<1>(swapList[randomSwap]);
				s.insert(swapIn);
				s.erase(swapOut);
				tabu_set.insert(swapOut);
				uniform_int_distribution<int> tabuTimer(1, swap.size());
				tabu_list[swapOut] = tabuTimer(mersenneTwister) + 7;
			}
			else {
				uniform_int_distribution<int> deleteDist(0, s.size() - 1);
				int randomDelete = deleteDist(mersenneTwister);
				vector<int> sList(s.begin(), s.end());
				int deleteVertex = sList[randomDelete];
				s.erase(deleteVertex);
				tabu_set.insert(deleteVertex);
				tabu_list[deleteVertex] = 7;
			}
			unimprovedCtr++;
			iterationCtr++;
			updateTabu(&tabu_set, &tabu_list);
			if (s.size() > localBest.size()) {
				unimprovedCtr = 0;
				localBest = s;
				cout << "Found new local best: " << localBest.size() << endl;
			}
		}
		if (localBest.size() > currMaxClique.size()) {
			currMaxClique = localBest;
			cout << "Found new currMax: " << currMaxClique.size() << " isClique: " << (isClique(currMaxClique, graph) ? "true" : "false")<< endl;
			vector<int> currMaxList(currMaxClique.begin(), currMaxClique.end());
			cout << "Clique: ";
			for (int i : currMaxList) {
				cout << i << " ";
			}
			cout << endl;
		}
	}
	return currMaxClique;
}

int main() {
	cout << "start " << endl;
	vector<vector<bool>> adjacencyMatrix = fromInputFile("MANN_a81.clq");
	cout << "input read " << endl;
	set<int> maxClique = approxMaxClique(adjacencyMatrix, 4000, 1000000);
	cout << "maxClique: " << maxClique.size() << " isClique: " << (isClique(maxClique, adjacencyMatrix) ? "true" : "false") << endl;
	char l;
	cin >> l;
	return 0;
}