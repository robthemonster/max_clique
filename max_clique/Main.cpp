#define _CRT_SECURE_NO_WARNINGS

#include <fstream>
#include <sstream>
#include <random>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <ctime>
#include <chrono>


using namespace std;

const int ADD = 0, SWAP = 1, REMOVE = 2;
const int MAX_ELITE_SOLUTIONS = 30;
const int NUM_ITERATIONS = 100;


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
	int * tiedDeltas;
	int  * tiedDeltasNeighborhoods; //tiedDeltasNeighborhoods[i] \in {0,1,2} correspond to add/swap/delete
	int tiedDeltasSize = 0;
	int * tiedDeltasTabu;
	int * tiedTabuNeighborhoods;
	int tiedDeltasTabuSize = 0;
	int * tabuList; //list of tabu vertices
	int * tabuMap; //corresponding map to the vertex number of a given tabu list entry
	bool * tabuContains; //map indicating whether a given vertex is contained in the tabu list
	int tabuListSize = 0;
	int numVertices;
	int weightMod;
	chrono::high_resolution_clock::time_point startTime;
	double startClock;
	long long iterationCtr;
	long long iterationTotal;
	long long solutionIterationCounts[NUM_ITERATIONS];
	long long solutionWeights[NUM_ITERATIONS];
	long long solutionSizes[NUM_ITERATIONS];
	double solutionTimeTaken[NUM_ITERATIONS];
	bool solutionWasClique[NUM_ITERATIONS];
	int currIt;
	int bestKnown;
	int maxUnimproved;

	int successes = 0;

	int ** eliteSet;
	int * eliteSetSize;
	int eliteMinQuality = INT32_MIN;

	const int MAX_ITER = 100000000;

	//map<set<int>, int> eliteSolutions;

	random_device rd;
	mt19937 mersenneTwister(rd());

	void parseGraphFromFile(string);
	void printResultsToFile(string);
	void runHeuristic(long long, long long);
	void performLocalSearch(bool);
	void performPathRelinking();
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
	int getRandomInt(int, int);
	bool selectBestDelta();
	void decrementTabu();
	bool isBestAClique();

	//long setWeight(set<int> clique) {
	//	long sum = 0;
	//	for (set<int>::iterator it = clique.begin(); it != clique.end(); it++) {
	//		sum += weights[*it];
	//	}
	//	return sum;
	//}

	//void setCurrentSolution(set<int> solutionSet) {
	//	solutionSize = 0;
	//	solutionWeight = 0;
	//	tabuListSize = 0;
	//	addListSize = 0;
	//	swapListSize = 0;
	//	fill(tabuContains, tabuContains + numVertices + 1, false);
	//	fill(solutionContains, solutionContains + numVertices + 1, false);
	//	for (int i = 1; i <= numVertices; i++) {
	//		numInSolutionNotConnectedTo[i] = 0;
	//		solutionContains[i] = false;
	//		addList[i - 1] = i;
	//		indexMap[i] = i - 1;
	//	}
	//	addListSize = numVertices;
	//	for (set<int>::iterator it = solutionSet.begin(); it != solutionSet.end(); it++) {
	//		addToSolution(*it);
	//	}
	//}

	//set<int> getNextMoveTowards(set<int> curr, set<int> guide) {
	//	set<int> intersection;
	//	set_intersection(curr.begin(), curr.end(),
	//		guide.begin(), guide.end(),
	//		inserter(intersection, intersection.begin()));
	//	set<int> currGuideDiff; // curr - guide
	//	set_difference(curr.begin(), curr.end(),
	//		guide.begin(), guide.end(),
	//		inserter(currGuideDiff, currGuideDiff.begin()));
	//	set<int> guideCurrDiff; //guide - curr
	//	set_difference(guide.begin(), guide.end(),
	//		curr.begin(), curr.end(),
	//		inserter(guideCurrDiff, guideCurrDiff.begin()));
	//	map<long, vector<int>> numDisconnected;
	//	if (guideCurrDiff.size() > 0) {
	//		for (set<int>::iterator it = guideCurrDiff.begin(); it != guideCurrDiff.end(); it++) {
	//			int notInCurr = *it;
	//			int disconnected = 0;
	//			long difference = weights[notInCurr];
	//			for (set<int>::iterator jt = currGuideDiff.begin(); jt != currGuideDiff.end(); jt++) {
	//				int notInGuide = *jt;
	//				if (!graph[notInCurr][notInGuide]) {
	//					disconnected++;
	//					difference -=  weights[notInGuide];
	//				}
	//			}
	//			numDisconnected[difference].push_back(notInCurr);
	//		}
	//	}
	//	if (numDisconnected.size() > 0) {
	//		int randomIndex = getRandomInt(0, numDisconnected.size() - 1);
	//		map<long, vector<int>>::iterator it = numDisconnected.begin();
	//		for (int i = 0; i < randomIndex; i++) {
	//			it++;
	//		}
	//		vector<int> minDisc = numDisconnected.rbegin()->second;
	//		int vertex = minDisc[getRandomInt(0, minDisc.size() - 1)];
	//		intersection.insert(vertex);
	//		for (set<int>::iterator it = currGuideDiff.begin(); it != currGuideDiff.end(); it++) {
	//			int otherVertex = *it;
	//			if (graph[vertex][otherVertex]) {
	//				intersection.insert(otherVertex);
	//			}
	//		}
	//	}
	//	return intersection;
	//}

	int getRandomInt(int min, int max) {
		return min + (rand() % (max - min + 1));
		//uniform_int_distribution<int> dist(min, max);
		//return dist(mersenneTwister);
	}

	int main2(int argc, char **argv) {
		srand((unsigned)time(NULL));
		char * fileName = argv[1];
		bestKnown = stoi(argv[2]);
		weightMod = stoi(argv[3]);
		maxUnimproved = stoi(argv[4]);
		parseGraphFromFile(fileName);
		long long weightTotal = 0;
		iterationTotal = 0;
		long iterationSum = 0;
		for (currIt = 0; currIt < NUM_ITERATIONS; currIt++) {
			runHeuristic(maxUnimproved, (MAX_ITER / maxUnimproved) + 1);
			solutionSizes[currIt] = bestSolutionSize;
			solutionWeights[currIt] = bestSolutionWeight;
			solutionWasClique[currIt] = isBestAClique();
			cout << (currIt + 1) << ") Best solution found: " << bestSolutionSize << " Weight: " << solutionWeights[currIt] << " isClique: " << (solutionWasClique[currIt] ? "true" : "false") << " Iteration: " << solutionIterationCounts[currIt] << endl;
			long weight = 0;
			cout << "[";
			for (int j = 0; j < bestSolutionSize; j++) {
				cout << bestSolution[j] << " ";
				weight += weights[bestSolution[j]];
			}
			cout << "]" << endl;
			cout << "Weight: " << weight << endl;
			if (weight >= bestKnown) {
				successes++;
			}
			iterationSum += solutionIterationCounts[currIt];
			weightTotal += weight;
			localBestSize = 0;
			localBestWeight = 0;
			bestSolutionSize = 0;
			bestSolutionWeight = 0;
		}
		printResultsToFile(fileName);
		deallocateGraph();
		char c;
		cin >> c;
		return -1;
	}

	bool isBestAClique() {
		for (int i = 0; i < bestSolutionSize; i++) {
			for (int j = 0; j < bestSolutionSize; j++) {
				if (i != j) {
					if (!graph[bestSolution[i]][bestSolution[j]]) {
						return false;
					}
				}
			}
		}
		return true;
	}

	void runHeuristic(long long maxUnimproved, long long maxIterations) {
		iterationCtr = 0;
		//startTime = chrono::high_resolution_clock::now();
		startClock = clock();
		iterationTotal = 0;
		bestSolutionSize = 0;
		bestSolutionWeight = INT32_MIN;
		//eliteMinQuality = INT32_MIN;
		//eliteSolutions.clear();
		while (iterationCtr < maxIterations) {
			setInitialSolution();
			performLocalSearch(false);
			if (bestSolutionWeight == bestKnown) {
				return;
			}
		}
		//cout << "Current best : "<< bestSolutionWeight << " Begin path relinking" << endl;
		//performPathRelinking();
	}

	void performLocalSearch(bool pathRelinkingMode)
	{

		localBestWeight = INT32_MIN;
		localBestSize = 0;
		long long unimprovedCtr = 0;
		while (unimprovedCtr < maxUnimproved) {
			if (selectBestDelta()) {
				unimprovedCtr++;
				decrementTabu();
				iterationTotal++;
				if (solutionWeight > localBestWeight) {
					for (int i = 0; i < solutionSize; i++) {
						localBest[i] = solution[i];
					}
					localBestSize = solutionSize;
					localBestWeight = solutionWeight;

					//if (pathRelinkingMode) {
					//	unimprovedCtr = 0;
					//}
					/*if (localBestWeight > eliteMinQuality) {
						set<int> clique;
						for (int i = 0; i < localBestSize; i++) {
							clique.insert(localBest[i]);
						}
						eliteSolutions[clique] = localBestWeight;
						if (eliteSolutions.size() > MAX_ELITE_SOLUTIONS) {
							int min = INT32_MAX;
							set<int> minKey;
							for (map<set<int>, int>::iterator it = eliteSolutions.begin(); it != eliteSolutions.end(); it++) {
								if (it->second < min) {
									min = it->second;
									minKey = it->first;
								}
							}
							eliteSolutions.erase(minKey);
							eliteMinQuality = INT32_MAX;
							for (map<set<int>, int>::iterator it = eliteSolutions.begin(); it != eliteSolutions.end(); it++) {
								if (it->second < eliteMinQuality) {
									eliteMinQuality = it->second;
								}
							}
						}
					}*/
				}
				if (/*!pathRelinkingMode &&*/ solutionWeight == bestKnown) {
					break;
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
			solutionIterationCounts[currIt] = iterationTotal;
			//if (!pathRelinkingMode) {
				//long long diff = chrono::duration_cast<chrono::milliseconds>(chrono::high_resolution_clock::now() - startTime).count();
			double diff = clock() - startClock;
			solutionTimeTaken[currIt] = diff;
			if (bestSolutionWeight == bestKnown) {
				return;
			}
			//}
		}

	}

	//void performPathRelinking()
	//{
	//	long currentBest = bestSolutionWeight;
	//	//map<set<int>, int> currEliteSet;
	//	int round = 0;
	//	do {
	//		round++;
	//		long before = bestSolutionWeight;
	//		int itCtr = 0;
	//		int jtCtr = 0;
	//		currEliteSet = map<set<int>, int>(eliteSolutions);
	//		for (map<set<int>, int>::iterator it = currEliteSet.begin(); it != currEliteSet.end(); it++) {
	//			jtCtr = itCtr;
	//			itCtr++;
	//			for (map<set<int>, int>::iterator jt = currEliteSet.begin(); jt != currEliteSet.end(); jt++) {
	//				jtCtr++;
	//				if (it != jt) {
	//					set<int> currSet = it->first;
	//					set<int> guide = jt->first;
	//					int stepCtr = 0;
	//					while (currSet != guide) {
	//						currSet = getNextMoveTowards(currSet, guide);
	//						stepCtr++;
	//						setCurrentSolution(currSet);
	//						performLocalSearch(true);
	//						if (localBestWeight > currentBest) {
	//							currentBest = localBestWeight;
	//						}
	//					}
	//				}
	//			}
	//		}
	//		cout << "Round " << round << " Before : " << before << " Best after path relinking: " << currentBest << endl;
	//	} while (eliteSolutions != currEliteSet);
	//}



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
		int greatestDelta = INT32_MIN;
		int greatestTabuDelta = INT32_MIN;
		tiedDeltasSize = 0;
		tiedDeltasTabuSize = 0;
		for (int i = 0; i < addListSize; i++) {
			bool tabu = tabuContains[addList[i]];
			if (!tabu) {
				if (weights[addList[i]] > greatestDelta) {
					greatestDelta = weights[addList[i]];
					tiedDeltasSize = 0;
					tiedDeltas[tiedDeltasSize] = i;
					tiedDeltasNeighborhoods[tiedDeltasSize] = ADD;
					tiedDeltasSize++;
				}
				else if (weights[addList[i]] == greatestDelta) {
					tiedDeltas[tiedDeltasSize] = i;
					tiedDeltasNeighborhoods[tiedDeltasSize] = ADD;
					tiedDeltasSize++;
				}
			}
			else {
				if (weights[addList[i]] > greatestTabuDelta) {
					greatestTabuDelta = weights[addList[i]];
					tiedDeltasTabuSize = 0;
					tiedDeltasTabu[tiedDeltasTabuSize] = i;
					tiedTabuNeighborhoods[tiedDeltasTabuSize] = ADD;
					tiedDeltasTabuSize++;
				}
				else if (weights[addList[i]] == greatestTabuDelta) {
					tiedDeltasTabu[tiedDeltasTabuSize] = i;
					tiedTabuNeighborhoods[tiedDeltasTabuSize] = ADD;
					tiedDeltasTabuSize++;
				}
			}
		}

		for (int i = 0; i < swapListSize; i++) {
			bool tabu = tabuContains[swapList[i]];
			int swapDelta = weights[swapList[i]] - weights[swapBuddy[swapList[i]]];
			if (!tabu) {
				if (swapDelta > greatestDelta) {
					greatestDelta = swapDelta;
					tiedDeltasSize = 0;
					tiedDeltas[tiedDeltasSize] = i;
					tiedDeltasNeighborhoods[tiedDeltasSize] = SWAP;
					tiedDeltasSize++;
				}
				else if (swapDelta == greatestDelta) {
					tiedDeltas[tiedDeltasSize] = i;
					tiedDeltasNeighborhoods[tiedDeltasSize] = SWAP;
					tiedDeltasSize++;
				}
			}
			else {
				if (swapDelta > greatestTabuDelta) {
					greatestTabuDelta = swapDelta;
					tiedDeltasTabuSize = 0;
					tiedDeltasTabu[tiedDeltasTabuSize] = i;
					tiedTabuNeighborhoods[tiedDeltasTabuSize] = SWAP;
					tiedDeltasTabuSize++;
				}
				else if (swapDelta == greatestTabuDelta) {
					tiedDeltasTabu[tiedDeltasTabuSize] = i;
					tiedTabuNeighborhoods[tiedDeltasTabuSize] = SWAP;
					tiedDeltasTabuSize++;
				}
			}
		}
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
		if (tiedDeltasTabuSize > 0 && greatestTabuDelta > greatestDelta && solutionWeight + greatestTabuDelta > localBestWeight) {
			int randomDeltaIndex = getRandomInt(0, tiedDeltasTabuSize - 1);
			if (tiedTabuNeighborhoods[randomDeltaIndex] == ADD) {
				int vertex = addList[tiedDeltasTabu[randomDeltaIndex]];
				addToSolution(vertex);
			}
			else if (tiedTabuNeighborhoods[randomDeltaIndex] == SWAP) {
				int vertex = swapList[tiedDeltasTabu[randomDeltaIndex]];
				swapIntoSolution(vertex);
			}
		}
		else if (tiedDeltasSize > 0) {
			int randomDeltaIndex = getRandomInt(0, tiedDeltasSize - 1);
			if (tiedDeltasNeighborhoods[randomDeltaIndex] == ADD) {
				int vertex = addList[tiedDeltas[randomDeltaIndex]];
				addToSolution(vertex);
			}
			else if (tiedDeltasNeighborhoods[randomDeltaIndex] == SWAP) {
				int vertex = swapList[tiedDeltas[randomDeltaIndex]];
				int swapBud = swapBuddy[vertex];
				swapIntoSolution(vertex);
				int timer = getRandomInt(1, swapListSize);
				tabuList[tabuListSize] = timer + 7;
				tabuMap[tabuListSize] = swapBud;
				tabuListSize++;
				tabuContains[swapBud] = true;
			}
			else {
				int vertex = solution[tiedDeltas[randomDeltaIndex]];
				removeFromSolution(tiedDeltas[randomDeltaIndex]);
				tabuList[tabuListSize] = 7;
				tabuMap[tabuListSize] = vertex;
				tabuListSize++;
				tabuContains[vertex] = true;
			}
		}
		else {
			return false;
		}
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
		int randomVertex = getRandomInt(1, numVertices);
		addToSolution(randomVertex);
		iterationTotal++;
		while (addListSize > 0) {
			int randomAddIndex = getRandomInt(0, addListSize - 1);
			addToSolution(addList[randomAddIndex]);
			iterationTotal++;
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
			}
			else if (numDisconnected == 0 && !solutionContains[vertex]) {
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
		indexMap[addList[index]] = index;

	}

	void removeFromSwap(int vertexToRemove)
	{
		int index = indexMap[vertexToRemove];
		swapList[index] = swapList[swapListSize - 1];
		swapListSize--;
		indexMap[swapList[index]] = index;

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
								weights[i] = (i % weightMod) + 1;
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
							tiedDeltasTabu = new int[numVertices];
							tiedTabuNeighborhoods = new int[numVertices];
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

	void printResultsToFile(string filename) {
		string outputFileName = filename.substr(0, filename.size() - 4);
		char * str;
		time_t t = chrono::system_clock::to_time_t(chrono::system_clock::now());
		str = ctime(&t);
		for (int i = 0; i < 100; i++) {
			if (str[i] == '\n')
			{
				break;
			}
			if (str[i] == ':') {
				outputFileName += ' ';
				continue;
			}
			outputFileName += str[i];
		}
		outputFileName += ".out";
		ofstream output;
		output.open(outputFileName);

		for (int i = 0; i < NUM_ITERATIONS; i++) {
			output << "Weight: " << solutionWeights[i] << " Iterations: " << solutionIterationCounts[i] << " Size: " <<
				solutionSizes[i] << " Time: " << solutionTimeTaken[i] << " isClique: " << (solutionWasClique[i] ? "true" : "false") << endl;
		}
		long long averageWeight, tiedIterationAvg, successes, tiedSize;
		double tiedAverageTimeSeconds = 0;
		tiedAverageTimeSeconds = averageWeight = tiedIterationAvg = successes = tiedSize = 0;
		int best = -1;
		int numTied = 0;
		for (int i = 0; i < NUM_ITERATIONS; i++) {
			if (solutionWeights[i] > best) {
				best = solutionWeights[i];
				tiedSize = solutionSizes[i];
				tiedIterationAvg = solutionIterationCounts[i];
				numTied = 0;
			}
			if (solutionWeights[i] == best) {
				tiedIterationAvg += solutionIterationCounts[i];
				tiedAverageTimeSeconds += solutionTimeTaken[i];
				numTied++;
			}
		}
		for (int i = 0; i < NUM_ITERATIONS; i++) {
			averageWeight += solutionWeights[i];
		}
		tiedIterationAvg = (double(tiedIterationAvg)) / numTied;
		averageWeight = (double(averageWeight)) / NUM_ITERATIONS;
		tiedAverageTimeSeconds = (double(tiedAverageTimeSeconds)) / (numTied * 1000);
		output << "Best soution weight found: " << best << endl;
		output << "Average Weight: " << averageWeight << " Successes: " << numTied <<
			" Tied Size: " << tiedSize << " Tied iteration average: " << tiedIterationAvg <<
			" Average Time (seconds): " << tiedAverageTimeSeconds << endl;
		output.close();
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
