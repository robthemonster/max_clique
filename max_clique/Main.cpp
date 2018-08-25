#include <istream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <set>
#include <random>
#include <time.h>

using namespace std;

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
				while (getline(stringStream, curr, ' ')) {
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
				while (getline(stringStream, curr, ' ')) {
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

int main() {
	vector<vector<bool>> adjacencyMatrix = fromInputFile("input.in");
	set<int> s = getInitialSolution(adjacencyMatrix);
	return 0;
}