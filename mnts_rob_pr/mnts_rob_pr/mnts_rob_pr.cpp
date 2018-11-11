/*  Multi-neighborhood tabu search for the maximum weight clique problem
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*
* This program demonstrates the use of the heuristic algorithm for solving
* the problem of the maximum weight clique problem. For more information about this algorithm,
* please visit the website http://www.info.univ-angers.fr/pub/hao/mnts_clique.html or email to: wu@info-univ.angers.fr.
*/
#pragma warning(disable : 4996)
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <string.h>
#include <time.h>
#include <ctime>
#include <vector>
#include <string.h>

using namespace std;

	char * File_Name;
	int **adjacencyMatrix;   // adjacent matrix
	int *inCurrSolution;
	int *numOfVerticesInSolutionNotConnectedTo;
	int *indices;
	int *tabuin;
	int numVertices;
	int *notConnectedLength;
	int **notConnected;
	int *currSolution;
	int currSolutionLength;
	int *localBestSolution;
	int numVerticesTimesSizeOfInt;
	int *adders;
	int *swappers;
	int *weights;
	int *swapBuddy;
	int addersLength;
	int swappersLength;
	int *tiedCandidatesTabuIndex;
	int iterationCtr;
	int TABUL = 7;
	int currSolutionQuality;
	int localBestSolutionQuality;
	int *tiedCandidateIndex;
	int bestKnownSolutionQuality;
	int iterationSolutionWasFoundOn;
	double starting_time, finishing_time, avg_time;
	int localBestSolutionLength = 0;
	int runBestLength;
	int Iteration[100];
	double time_used[100];
	int len_used[100];
	int solutionQuality[100];
	char outfilename[30];
	int maxUnimproved;
	int maxIterationsDividedByMaxUnimproved;
	int weightMod;

	int prIterations[100];
	double prTimeUsed[100];
	int prLenUsed[100];
	int prSolutionQuality[100];

	int prBestQuality = 0;
	int prBestLength = 0;
	double prStartTime = 0, prFinishTime = 0;
	int prIterationSolutionWasFoundOn = 0;

	const int ELITE_SET_CAPACITY = 25;
	int** eliteSet;
	int eliteSetSize = 0;
	int* eliteSetSizes;
	int * eliteSetWeights;
	int eliteSetMinWeight = INT32_MIN;

	int * initialSolution;
	int initialSolutionSize = 0;
	int * guidingSolution;
	int guidingSolutionSize = 0;
	bool * guidingContains;

	int * workingSolution;
	int workingSolutionSize = 0;
	bool * workingContains;

	int * tempWorkingSolution;
	int tempWorkingSolutionSize = 0;

	int * workingMinusGuiding;
	int workingMinusGuidingSize = 0;
	bool * workingMinusGuidingContains;

	int * setDifferenceIndexMap;


	int * guidingMinusWorking;
	int guidingMinusWorkingSize = 0;
	bool * guidingMinusWorkingContains;

	int * tiedPRSwapIns;
	int tiedPRSwapInsSize = 0;

	int * s2Contains;


	bool isClique(int * list, int size) {
		for (int i = 0; i < size; i++) {
			for (int j = 0; j < size; j++) {
				if (i != j) {
					if (list[i] == list[j]) {
						cout << "oops" << endl;
					}
					if (adjacencyMatrix[list[i]][list[j]] == 1) {
						return false;
					}
				}
			}
		}
		return true;
	}

	// section 0, initiaze
	void Initializing()
	{
		ifstream FIC;
		FIC.open(File_Name);
		if (FIC.fail())
		{
			cout << "### Erreur open, File_Name " << File_Name << endl;
			getchar();
			exit(0);
		}
		char StrReading[100];
		FIC >> StrReading;
		if (FIC.eof())
		{
			cout << "### Error open, File_Name " << File_Name << endl;
			exit(0);
		}
		int nb_vtx = 0, nb_edg = -1, max_edg = 0;
		int x1, x2;
		while (!FIC.eof())
		{
			if (strcmp(StrReading, "p") == 0)
			{
				FIC >> StrReading;
				FIC >> numVertices >> nb_edg;
				cout << "Number of vertexes = " << numVertices << endl;
				cout << "Number of edges = " << nb_edg << endl;

				inCurrSolution = new int[numVertices];
				numOfVerticesInSolutionNotConnectedTo = new int[numVertices];
				indices = new int[numVertices];
				tabuin = new int[numVertices];
				notConnectedLength = new int[numVertices];
				adders = new int[numVertices];
				swappers = new int[numVertices];
				tiedCandidatesTabuIndex = new int[numVertices];
				weights = new int[numVertices];
				swapBuddy = new int[numVertices];
				tiedCandidateIndex = new int[numVertices];
				notConnected = new int*[numVertices];
				eliteSet = new int*[ELITE_SET_CAPACITY];
				eliteSetSizes = new int[ELITE_SET_CAPACITY];
				eliteSetWeights = new int[ELITE_SET_CAPACITY];
				currSolution = new int[2000];
				localBestSolution = new int[2000];

				initialSolution = new int[2000];
				guidingSolution = new int[2000];
				workingSolution = new int[2000];
				tempWorkingSolution = new int[2000];
				workingMinusGuiding = new int[2000];
				guidingMinusWorking = new int[2000];
				workingMinusGuidingContains = new bool[numVertices];
				guidingMinusWorkingContains = new bool[numVertices];
				setDifferenceIndexMap = new int[numVertices];

				workingContains = new bool[numVertices];
				guidingContains = new bool[numVertices];
				tiedPRSwapIns = new int[numVertices];

				s2Contains = new int[numVertices];
							   			

				for (int x = 0; x < ELITE_SET_CAPACITY; x++) {
					eliteSet[x] = new int[numVertices];
				}
				for (int x = 0; x < numVertices; x++)
				{
					inCurrSolution[x] = x;
					indices[x] = x;
				}
				adjacencyMatrix = new int*[numVertices];

				for (int x = 0; x < numVertices; x++)
				{
					adjacencyMatrix[x] = new int[numVertices];
					notConnected[x] = new int[numVertices];
				}

				for (int x = 0; x < numVertices; x++)
					for (int y = 0; y < numVertices; y++)
						adjacencyMatrix[x][y] = 0;
			}
			if (strcmp(StrReading, "e") == 0)
			{
				FIC >> x1 >> x2;
				x1--; x2--;
				if (x1 < 0 || x2 < 0 || x1 >= numVertices || x2 >= numVertices)
				{
					cout << "### Error of node : x1="
						<< x1 << ", x2=" << x2 << endl;
					exit(0);
				}
				adjacencyMatrix[x1][x2] = adjacencyMatrix[x2][x1] = 1;
				max_edg++;

			}
			FIC >> StrReading;
		}
		cout << "Density = " << (float)max_edg / (numVertices*(numVertices - 1)) << endl;

		if (0 && max_edg != nb_edg)
		{
			cout << "### Error de lecture du graphe, nbre aretes : annonce="
				<< nb_edg << ", lu=" << max_edg << endl;
			exit(0);
		}

		for (int x = 0; x < numVertices; x++)
			for (int y = 0; y < numVertices; y++)
				adjacencyMatrix[x][y] = 1 - adjacencyMatrix[x][y]; //flipping values from 0 to 1 and vice versa. now 0 == connected, 1 == disconnected

		for (int x = 0; x < numVertices; x++)
			adjacencyMatrix[x][x] = 0; //connected to self

		for (int x = 0; x < numVertices; x++)
		{
			notConnectedLength[x] = 0;
			for (int y = 0; y < numVertices; y++)
			{
				if (adjacencyMatrix[x][y] == 1)
				{
					notConnected[x][notConnectedLength[x]] = y;
					notConnectedLength[x]++;
				}
			}
		}

		for (int x = 0; x < numVertices; x++)
		{
			weights[x] = (x + 1) % weightMod + 1;
			swapBuddy[x] = 0;
		}

		FIC.close();
	}

	int randomInt(int n)
	{
		int randomInt = rand() % n;
		return randomInt;
	}

	void resetLists()
	{
		int i;
		memset(inCurrSolution, 0, numVerticesTimesSizeOfInt);
		memset(numOfVerticesInSolutionNotConnectedTo, 0, numVerticesTimesSizeOfInt);
		memset(indices, 0, numVerticesTimesSizeOfInt);
		memset(tabuin, 0, numVerticesTimesSizeOfInt);
		for (i = 0; i < numVertices; i++)
		{
			adders[i] = i;
			indices[i] = i;
		}
		addersLength = numVertices;
		swappersLength = 0;
		currSolutionLength = 0;
		currSolutionQuality = 0;
		localBestSolutionQuality = 0;
	}

	int selectNextVertexForInitialSolution()
	{
		int i, k, numNonTabu;
		numNonTabu = 0;
		if (addersLength > 30)
		{
			k = randomInt(addersLength);
			return k;
		}
		for (i = 0; i < addersLength; i++)
		{
			k = adders[i];
			if (tabuin[k] <= iterationCtr)
				tiedCandidatesTabuIndex[numNonTabu++] = i;
		}
		if (numNonTabu == 0)
			return -1;
		else
		{
			k = randomInt(numNonTabu);
			k = tiedCandidatesTabuIndex[k];
			return k;
		}
	}

	int WSelectBestFromAdd()
	{
		int i, k, tiedCandidatesLength, tiedCandidatesTabuLength, greatestDelta, greatestTabuDelta;
		tiedCandidatesLength = 0;
		tiedCandidatesTabuLength = 0;
		greatestDelta = 0;
		greatestTabuDelta = 0;

		for (i = 0; i < addersLength; i++)
		{
			k = adders[i];
			if (tabuin[k] <= iterationCtr)
			{
				if (weights[k] > greatestDelta)
				{
					tiedCandidatesLength = 0;
					greatestDelta = weights[k];
					tiedCandidateIndex[tiedCandidatesLength++] = i;
				}
				else if (weights[k] >= greatestDelta)
				{
					tiedCandidateIndex[tiedCandidatesLength++] = i;
				}
			}
			else
			{
				if (weights[k] > greatestTabuDelta)
				{
					tiedCandidatesTabuLength = 0;
					greatestTabuDelta = weights[k];
					tiedCandidatesTabuIndex[tiedCandidatesTabuLength++] = i;
				}
				else if (weights[k] >= greatestTabuDelta)
				{
					tiedCandidatesTabuIndex[tiedCandidatesTabuLength++] = i;
				}
			}
		}

		if ((tiedCandidatesTabuLength > 0) && (greatestTabuDelta > greatestDelta) && ((greatestTabuDelta + currSolutionQuality) > localBestSolutionQuality))
		{
			k = randomInt(tiedCandidatesTabuLength);
			k = tiedCandidatesTabuIndex[k];
			return k;
		}
		else if (tiedCandidatesLength > 0)
		{
			k = randomInt(tiedCandidatesLength);
			k = tiedCandidateIndex[k];
			return k;
		}
		else
		{
			return -1;
		}
	}

	int addToCurrSolution(int SelN)
	{
		int i, nIndex, m, n, last;

		m = adders[SelN];
		currSolution[currSolutionLength++] = m;
		inCurrSolution[m] = 1;
		currSolutionQuality = currSolutionQuality + weights[m];

		addersLength--;
		last = adders[addersLength];
		nIndex = indices[m];
		adders[nIndex] = last;
		indices[last] = nIndex;

		for (i = 0; i < notConnectedLength[m]; i++)
		{
			n = notConnected[m][i];
			numOfVerticesInSolutionNotConnectedTo[n]++;
			if (numOfVerticesInSolutionNotConnectedTo[n] == 1)
			{
				nIndex = indices[n];
				addersLength--;
				last = adders[addersLength];
				adders[nIndex] = last;
				indices[last] = nIndex; //swap to remove

				swappers[swappersLength] = n;
				indices[n] = swappersLength;
				swappersLength++;
				swapBuddy[n] = m;
			}
			else if (numOfVerticesInSolutionNotConnectedTo[n] == 2)
			{
				swappersLength--;
				last = swappers[swappersLength];
				nIndex = indices[n];
				swappers[nIndex] = last;
				indices[last] = nIndex;
			}
		}

		if (currSolutionQuality > localBestSolutionQuality)
		{
			localBestSolutionQuality = currSolutionQuality;
			localBestSolutionLength = currSolutionLength;
			for (int i = 0; i < currSolutionLength; i++) {
				localBestSolution[i] = currSolution[i];
			}
		}

		return 1;
	}

	int WSelectBestFromSwappy()
	{
		int i, j, k, l, tiedCandidatesLength, tiedCandidatesTabuLength, swapDelta, greatestDelta, tabuGreatestDelta, swapIn, swapOut;
		tiedCandidatesLength = 0;
		tiedCandidatesTabuLength = 0;
		greatestDelta = -1000000;
		tabuGreatestDelta = -1000000;
		l = 0;
		for (i = 0; i < swappersLength; i++)
		{
			swapIn = swappers[i];
			swapOut = swapBuddy[swapIn];
			if ((inCurrSolution[swapOut] == 1) && (adjacencyMatrix[swapIn][swapOut] == 1))
				l++;
			else
			{
				for (j = 0; j < currSolutionLength; j++)
				{
					k = currSolution[j];
					if (adjacencyMatrix[swapIn][k] == 1)
						break;
				}
				swapBuddy[swapIn] = k;
			}
		}
		for (i = 0; i < swappersLength; i++)
		{
			swapIn = swappers[i];
			swapOut = swapBuddy[swapIn];
			swapDelta = weights[swapIn] - weights[swapOut];
			if (tabuin[swapIn] <= iterationCtr)
			{
				if (swapDelta > greatestDelta)
				{
					tiedCandidatesLength = 0;
					greatestDelta = swapDelta;
					tiedCandidateIndex[tiedCandidatesLength++] = i;
				}
				else if (swapDelta >= greatestDelta)
				{
					tiedCandidateIndex[tiedCandidatesLength++] = i;
				}
			}
			else
			{
				if (swapDelta > tabuGreatestDelta)
				{
					tiedCandidatesTabuLength = 0;
					tabuGreatestDelta = swapDelta;
					tiedCandidatesTabuIndex[tiedCandidatesTabuLength++] = i;
				}
				else if (swapDelta >= tabuGreatestDelta)
				{
					tiedCandidatesTabuIndex[tiedCandidatesTabuLength++] = i;
				}
			}
		}

		if ((tiedCandidatesTabuLength > 0) && (tabuGreatestDelta > greatestDelta) && ((tabuGreatestDelta + currSolutionQuality) > localBestSolutionQuality))
		{
			k = randomInt(tiedCandidatesTabuLength);
			k = tiedCandidatesTabuIndex[k];
			return k;
		}
		else if (tiedCandidatesLength > 0)
		{
			k = randomInt(tiedCandidatesLength);
			k = tiedCandidateIndex[k];
			return k;
		}
		else
		{
			return -1;
		}
	}

	int doTheSwappy(int SelN)
	{
		int i, swapIndexIn, vertexIn, vertexOut, disconnectedVertex, lastElementOfSwappers, indexOut;

		vertexIn = swappers[SelN];
		for (indexOut = 0; indexOut < currSolutionLength; indexOut++)
		{
			vertexOut = currSolution[indexOut];
			if (adjacencyMatrix[vertexOut][vertexIn] == 1)
				break;
		}

		currSolutionQuality = currSolutionQuality + weights[vertexIn] - weights[vertexOut];

		//the expand process, put m into the current independent set
		inCurrSolution[vertexIn] = 1;
		currSolution[currSolutionLength++] = vertexIn;
		//delete m from C1
		swapIndexIn = indices[vertexIn];
		swappersLength--;
		lastElementOfSwappers = swappers[swappersLength];
		swappers[swapIndexIn] = lastElementOfSwappers;
		indices[lastElementOfSwappers] = swapIndexIn;

		for (i = 0; i < notConnectedLength[vertexIn]; i++)
		{
			disconnectedVertex = notConnected[vertexIn][i];
			numOfVerticesInSolutionNotConnectedTo[disconnectedVertex]++;
			if ((numOfVerticesInSolutionNotConnectedTo[disconnectedVertex] == 1) && (inCurrSolution[disconnectedVertex] == 0))
			{
				swapIndexIn = indices[disconnectedVertex];
				addersLength--;
				lastElementOfSwappers = adders[addersLength];
				adders[swapIndexIn] = lastElementOfSwappers;
				indices[lastElementOfSwappers] = swapIndexIn;

				swappers[swappersLength] = disconnectedVertex;
				indices[disconnectedVertex] = swappersLength;
				swappersLength++;
				swapBuddy[disconnectedVertex] = vertexIn;
			}
			if (numOfVerticesInSolutionNotConnectedTo[disconnectedVertex] == 2)
			{
				swappersLength--;
				lastElementOfSwappers = swappers[swappersLength];
				swapIndexIn = indices[disconnectedVertex];
				swappers[swapIndexIn] = lastElementOfSwappers;
				indices[lastElementOfSwappers] = swapIndexIn;
			}
		}

		//the backtrack process, delete m1 from the current independent set
		inCurrSolution[vertexOut] = 0;
		tabuin[vertexOut] = iterationCtr + TABUL + randomInt(swappersLength + 2);
		currSolutionLength--;
		currSolution[indexOut] = currSolution[currSolutionLength];
		swappers[swappersLength] = vertexOut;
		indices[vertexOut] = swappersLength;
		swappersLength++;

		for (i = 0; i < notConnectedLength[vertexOut]; i++)
		{
			disconnectedVertex = notConnected[vertexOut][i];
			numOfVerticesInSolutionNotConnectedTo[disconnectedVertex]--;
			if ((numOfVerticesInSolutionNotConnectedTo[disconnectedVertex] == 0) && (inCurrSolution[disconnectedVertex] == 0))
			{
				swapIndexIn = indices[disconnectedVertex];
				swappersLength--;
				lastElementOfSwappers = swappers[swappersLength];
				swappers[swapIndexIn] = lastElementOfSwappers;
				indices[lastElementOfSwappers] = swapIndexIn;

				adders[addersLength] = disconnectedVertex;
				indices[disconnectedVertex] = addersLength;
				addersLength++;
			}
			else if (numOfVerticesInSolutionNotConnectedTo[disconnectedVertex] == 1)
			{
				swappers[swappersLength] = disconnectedVertex;
				indices[disconnectedVertex] = swappersLength;
				swappersLength++;
			}
		}

		if (currSolutionQuality > localBestSolutionQuality)
		{
			localBestSolutionQuality = currSolutionQuality;
			localBestSolutionLength = currSolutionLength;
			for (int i = 0; i < currSolutionLength; i++) {
				localBestSolution[i] = currSolution[i];
			}
		}
		return 1;
	}

	int findMinWeightInCurrSolution()
	{
		int i, k, tiedCounter;
		int minWeight = 5000000;
		tiedCounter = 0;
		for (i = 0; i < currSolutionLength; i++)
		{
			k = currSolution[i];
			if (weights[k] < minWeight)
			{
				tiedCounter = 0;
				minWeight = weights[k];
				tiedCandidateIndex[tiedCounter++] = i;
			}
			else if (weights[k] <= minWeight)
			{
				tiedCandidateIndex[tiedCounter++] = i;
			}
		}

		if (tiedCounter == 0)
			return -1;
		k = randomInt(tiedCounter);
		k = tiedCandidateIndex[k];
		return k;
	}

	int removeFromSolution()
	{
		int i, minVertex, disconnectedVertex, minIndex, swapIndex, last;
		minIndex = findMinWeightInCurrSolution();
		if (minIndex == -1)
			return -1;
		minVertex = currSolution[minIndex];
		currSolutionQuality = currSolutionQuality - weights[minVertex];
		inCurrSolution[minVertex] = 0;
		tabuin[minVertex] = iterationCtr + TABUL;
		currSolutionLength--;
		currSolution[minIndex] = currSolution[currSolutionLength];
		adders[addersLength] = minVertex;
		indices[minVertex] = addersLength;
		addersLength++;

		for (i = 0; i < notConnectedLength[minVertex]; i++)
		{
			disconnectedVertex = notConnected[minVertex][i];
			numOfVerticesInSolutionNotConnectedTo[disconnectedVertex]--;
			if ((numOfVerticesInSolutionNotConnectedTo[disconnectedVertex] == 0) && (inCurrSolution[disconnectedVertex] == 0))
			{
				swapIndex = indices[disconnectedVertex];
				swappersLength--;
				last = swappers[swappersLength];
				swappers[swapIndex] = last;
				indices[last] = swapIndex;

				adders[addersLength] = disconnectedVertex;
				indices[disconnectedVertex] = addersLength;
				addersLength++;
			}
			else if (numOfVerticesInSolutionNotConnectedTo[disconnectedVertex] == 1)
			{
				swappers[swappersLength] = disconnectedVertex;
				indices[disconnectedVertex] = swappersLength;
				swappersLength++;
			}
		}
	}

	int setInitialSolution() {
		int mysteriousL, bestToAdd;
		iterationCtr = 0;
		resetLists();
		while (1)
		{
			bestToAdd = selectNextVertexForInitialSolution();
			if (bestToAdd != -1)
			{
				mysteriousL = addToCurrSolution(bestToAdd);
				iterationCtr++;
				if (localBestSolutionQuality == bestKnownSolutionQuality)
					return localBestSolutionQuality;
			}
			else
				return localBestSolutionQuality;
		}
	}

	int performLocalSearch(int maxUnimproved, bool pathRelinking)
	{
		int k, mysteriousL, bestToAdd, bestToSwap, addDelta, swapDelta, deleteDelta, bestDeleteIndex, bestToDelete;
		int restartThreshold = INT32_MIN;
		int tmpMaxUnimproved = maxUnimproved;
		while (iterationCtr < maxUnimproved)
		{
			bestToAdd = WSelectBestFromAdd();
			bestToSwap = WSelectBestFromSwappy();
			if ((bestToAdd != -1) && (bestToSwap != -1))
			{
				addDelta = weights[adders[bestToAdd]];
				swapDelta = weights[swappers[bestToSwap]] - weights[swapBuddy[swappers[bestToSwap]]];

				if (addDelta > swapDelta)
				{
					mysteriousL = addToCurrSolution(bestToAdd);

					iterationCtr++;
					if (localBestSolutionQuality == bestKnownSolutionQuality && !pathRelinking)
						return localBestSolutionQuality;
				}
				else
				{
					mysteriousL = doTheSwappy(bestToSwap);
					if (localBestSolutionQuality == bestKnownSolutionQuality && !pathRelinking)
						return localBestSolutionQuality;
					iterationCtr++;
				}
			}
			else if ((bestToAdd != -1) && (bestToSwap == -1))
			{
				mysteriousL = addToCurrSolution(bestToAdd);
				if (localBestSolutionQuality == bestKnownSolutionQuality && !pathRelinking)
					return localBestSolutionQuality;

				iterationCtr++;
			}
			else if ((bestToAdd == -1) && (bestToSwap != -1))
			{
				bestDeleteIndex = findMinWeightInCurrSolution();
				bestToDelete = currSolution[bestDeleteIndex];
				swapDelta = weights[swappers[bestToSwap]] - weights[swapBuddy[swappers[bestToSwap]]];
				deleteDelta = -weights[bestToDelete];
				if (swapDelta > deleteDelta)
				{
					mysteriousL = doTheSwappy(bestToSwap);
					if (localBestSolutionQuality == bestKnownSolutionQuality && !pathRelinking)
						return localBestSolutionQuality;
					iterationCtr++;
				}
				else
				{
					k = removeFromSolution();
					if (k == -1)
						return localBestSolutionQuality;
					iterationCtr++;
				}
			}
			else if ((bestToAdd == -1) && (bestToSwap == -1))
			{
				k = removeFromSolution();
				if (k == -1)
					return localBestSolutionQuality;
				iterationCtr++;
			}
			if (pathRelinking && localBestSolutionQuality > restartThreshold) {
				//maxUnimproved = iterationCtr + tmpMaxUnimproved;
				restartThreshold = localBestSolutionQuality;
			}
		}

		return localBestSolutionQuality;
	}

	bool isEqual(int * s1, int size1, int * s2, int size2) {
		if (size1 != size2) {
			return false;
		}
		memset(s2Contains, false, numVertices * sizeof(bool));
		for (int i = 0; i < size2; i++) {
			s2Contains[s2[i]] = true;
		}
		for (int i = 0; i < size1; i++) {
			if (!s2Contains[s1[i]]) {
				return false;
			}
		}
		return true;
	}


	bool isUnique(int solution[], int size) {
		for (int i = 0; i < eliteSetSize; i++) {
			if (isEqual(eliteSet[i], eliteSetSizes[i], solution, size)) {
				return false;
			}
		}
		return true;

	}

	void printResultsToFile()
	{
		int i;
		FILE *fp;
		int len = strlen(File_Name);
		strcpy(outfilename, File_Name);
		outfilename[len] = '.';
		outfilename[len + 1] = 'o';
		outfilename[len + 2] = 'u';
		outfilename[len + 3] = 't';
		outfilename[len + 4] = '\0';

		fp = fopen(outfilename, "a+");
		for (i = 0; i < 100; i++)
		{
			fprintf(fp, "sum = %6d, iter = %6d, len = %5d,  time = %8lf \n", solutionQuality[i], Iteration[i], len_used[i], time_used[i]);
			fprintf(fp, "PR: sum = %6d, iter = %6d, len = %5d,  time = %8lf \n", prSolutionQuality[i], prIterations[i], prLenUsed[i], prTimeUsed[i]);
		}

		fprintf(fp, "\n\n the total information: \n");
		int wavg, tiedIterationAvg, lenbb;
		wavg = tiedIterationAvg = lenbb = 0;
		int bestSolutionQuality = 0;
		double tiedTimeAvg = 0;
		for (i = 0; i < 100; i++)
			if (solutionQuality[i] > bestSolutionQuality)
			{
				bestSolutionQuality = solutionQuality[i];
				lenbb = len_used[i];
			}

		int numTiedForBest = 0;
		fprintf(fp, "\n The best weight value for the maximum weighted problem is %6d \n", bestSolutionQuality);
		for (i = 0; i < 100; i++)
		{
			wavg = wavg + solutionQuality[i];
		}
		double twavg = (double(wavg)) / 100;
		for (i = 0; i < 100; i++)
			if (solutionQuality[i] >= bestSolutionQuality)
			{
				numTiedForBest++;
				tiedIterationAvg = tiedIterationAvg + Iteration[i];
				tiedTimeAvg = tiedTimeAvg + time_used[i];
			}

		tiedIterationAvg = int((double(tiedIterationAvg)) / numTiedForBest);
		tiedTimeAvg = tiedTimeAvg / (numTiedForBest * 1000);
		fprintf(fp, "avg_sum = %10lf, succes = %6d, len = %5d, avg_iter = %6d,  time = %8lf \n", twavg, numTiedForBest, lenbb, tiedIterationAvg, tiedTimeAvg);

		double totalImprovement = 0;
		double totalIterationsTaken = 0;
		int numImproved = 0;
		for (int i = 0; i < 100; i++) {
			totalImprovement += prSolutionQuality[i] - solutionQuality[i];
			totalIterationsTaken += prIterations[i];
			if (prSolutionQuality[i] > solutionQuality[i]) {
				numImproved++;
			}
		}

		fprintf(fp, "Avg improvment = %10lf, Num Improved = %6d, Iterations/Improvement = %10lf", totalImprovement / 100.0, numImproved, totalIterationsTaken/totalImprovement);
		
		fclose(fp);
		return;
	}
	void addLocalBestToEliteSet() {
		if (!isUnique(localBestSolution, localBestSolutionLength)) {
			return;
		}
		if (eliteSetSize >= ELITE_SET_CAPACITY) {
			int min = INT32_MAX;
			int minIndex = 0;
			for (int i = 0; i < eliteSetSize; i++) {
				if (eliteSetWeights[i] < min) {
					min = eliteSetWeights[i];
					minIndex = i;
				}
			}
			for (int j = 0; j < localBestSolutionLength; j++) {
				eliteSet[minIndex][j] = localBestSolution[j];
			}
			eliteSetSizes[minIndex] = localBestSolutionLength;
			eliteSetWeights[minIndex] = localBestSolutionQuality;
			eliteSetMinWeight = INT32_MAX;
			for (int i = 0; i < eliteSetSize; i++) {
				if (eliteSetWeights[i] < eliteSetMinWeight) {
					eliteSetMinWeight = eliteSetWeights[i];
				}
			}
		}
		else {
			for (int j = 0; j < localBestSolutionLength; j++) {
				eliteSet[eliteSetSize][j] = localBestSolution[j];
			}
			eliteSetSizes[eliteSetSize] = localBestSolutionLength;
			eliteSetWeights[eliteSetSize] = localBestSolutionQuality;
			eliteSetSize++;
		}
	}

	void calculateDifferences() {
		workingMinusGuidingSize = 0;
		guidingMinusWorkingSize = 0;
		memset(workingMinusGuidingContains, false, sizeof(bool) * numVertices);
		memset(guidingMinusWorkingContains, false, sizeof(bool) * numVertices);
		for (int i = 0; i < workingSolutionSize; i++) {
			if (!guidingContains[workingSolution[i]]) {
				workingMinusGuiding[workingMinusGuidingSize] = workingSolution[i];
				setDifferenceIndexMap[workingSolution[i]] = workingMinusGuidingSize;
				workingMinusGuidingContains[workingSolution[i]] = true;
				workingMinusGuidingSize++;
			}
		}
		for (int i = 0; i < guidingSolutionSize; i++) {
			if (!workingContains[guidingSolution[i]]) {
				guidingMinusWorking[guidingMinusWorkingSize] = guidingSolution[i];
				setDifferenceIndexMap[guidingSolution[i]] = guidingMinusWorkingSize;
				guidingMinusWorkingContains[guidingSolution[i]] = true;
				guidingMinusWorkingSize++;
			}
		}
	}

	void moveWorkingTowardsGuidingSmallestSize() {
		int minDisconnected = INT32_MAX;
		tiedPRSwapInsSize = 0;
		for (int i = 0; i < guidingMinusWorkingSize; i++) {
			int guidingVertex = guidingMinusWorking[i];
			int numDisconnected = 0;
			for (int j = 0; j < workingMinusGuidingSize; j++) {
				int workingVertex = workingMinusGuiding[j];
				if (adjacencyMatrix[guidingVertex][workingVertex] == 1) {
					numDisconnected++;
				}
			}
			if (numDisconnected < minDisconnected) {
				numDisconnected = minDisconnected;
				tiedPRSwapInsSize = 0;
				tiedPRSwapIns[tiedPRSwapInsSize] = guidingVertex;
				tiedPRSwapInsSize++;
			}
			else if (numDisconnected == minDisconnected) {
				tiedPRSwapIns[tiedPRSwapInsSize] = guidingVertex;
				tiedPRSwapInsSize++;
			}
		}

		int randomIndex = randomInt(tiedPRSwapInsSize);
		int swapInVertex = tiedPRSwapIns[randomIndex];
		tempWorkingSolutionSize = 0;
		for (int i = 0; i < workingSolutionSize; i++) {
			int vertex = workingSolution[i];
			if (workingMinusGuidingContains[vertex]) {
				if (adjacencyMatrix[swapInVertex][vertex] == 0) {
					tempWorkingSolution[tempWorkingSolutionSize] = vertex;
					tempWorkingSolutionSize++;
				}
				else {
					workingMinusGuidingContains[vertex] = false;
					int index = setDifferenceIndexMap[vertex];
					swap(workingMinusGuiding[index], workingMinusGuiding[workingMinusGuidingSize - 1]);
					setDifferenceIndexMap[workingMinusGuiding[index]] = index;
					workingMinusGuidingSize--;
				}
			}
			else {
				tempWorkingSolution[tempWorkingSolutionSize] = vertex;
				tempWorkingSolutionSize++;
			}
		}
		tempWorkingSolution[tempWorkingSolutionSize] = swapInVertex;
		tempWorkingSolutionSize++;

		guidingMinusWorkingContains[swapInVertex] = false;
		int index = setDifferenceIndexMap[swapInVertex];
		swap(guidingMinusWorking[index], guidingMinusWorking[guidingMinusWorkingSize - 1]);
		setDifferenceIndexMap[guidingMinusWorking[index]] = index;
		guidingMinusWorkingSize--;
		memset(workingContains, false, sizeof(bool) * numVertices);
		for (int i = 0; i < tempWorkingSolutionSize; i++) {
			workingSolution[i] = tempWorkingSolution[i];
			workingContains[tempWorkingSolution[i]] = true;
		}
		workingSolutionSize = tempWorkingSolutionSize;
	}

	void moveWorkingTowardsGuidingGreatestDelta() {
		int maxDelta = INT32_MIN;
		tiedPRSwapInsSize = 0;
		for (int i = 0; i < guidingMinusWorkingSize; i++) {
			int delta = weights[guidingMinusWorking[i]];
			int guidingVertex = guidingMinusWorking[i];
			for (int j = 0; j < workingMinusGuidingSize; j++) {
				int workingVertex = workingMinusGuiding[j];
				if (adjacencyMatrix[guidingVertex][workingVertex] == 1) {
					delta -= weights[workingVertex];
				}
			}
			if (delta > maxDelta) {
				maxDelta = delta;
				tiedPRSwapInsSize = 0;
				tiedPRSwapIns[tiedPRSwapInsSize] = guidingVertex;
				tiedPRSwapInsSize++;
			}
			else if (delta == maxDelta) {
				tiedPRSwapIns[tiedPRSwapInsSize] = guidingVertex;
				tiedPRSwapInsSize++;
			}
		}

		int randomIndex = randomInt(tiedPRSwapInsSize);
		int swapInVertex = tiedPRSwapIns[randomIndex];
		tempWorkingSolutionSize = 0;
		for (int i = 0; i < workingSolutionSize; i++) {
			int vertex = workingSolution[i];
			if (workingMinusGuidingContains[vertex]) {
				if (adjacencyMatrix[swapInVertex][vertex] == 0) {
					tempWorkingSolution[tempWorkingSolutionSize] = vertex;
					tempWorkingSolutionSize++;
				}
				else {
					workingMinusGuidingContains[vertex] = false;
					int index = setDifferenceIndexMap[vertex];
					swap(workingMinusGuiding[index], workingMinusGuiding[workingMinusGuidingSize - 1]);
					setDifferenceIndexMap[workingMinusGuiding[index]] = index;
					workingMinusGuidingSize--;
				}
			}
			else {
				tempWorkingSolution[tempWorkingSolutionSize] = vertex;
				tempWorkingSolutionSize++;
			}
		}
		tempWorkingSolution[tempWorkingSolutionSize] = swapInVertex;
		tempWorkingSolutionSize++;

		guidingMinusWorkingContains[swapInVertex] = false;
		int index = setDifferenceIndexMap[swapInVertex];
		swap(guidingMinusWorking[index], guidingMinusWorking[guidingMinusWorkingSize - 1]);
		setDifferenceIndexMap[guidingMinusWorking[index]] = index;
		guidingMinusWorkingSize--;
		memset(workingContains, false, sizeof(bool) * numVertices);
		for (int i = 0; i < tempWorkingSolutionSize; i++) {
			workingSolution[i] = tempWorkingSolution[i];
			workingContains[tempWorkingSolution[i]] = true;
		}
		workingSolutionSize = tempWorkingSolutionSize;
	}

	void moveWorkingTowardsGuidingRandom() {
		int randomIndex = randomInt(guidingMinusWorkingSize);
		int swapInVertex = guidingMinusWorking[randomIndex];
		tempWorkingSolutionSize = 0;
		for (int i = 0; i < workingSolutionSize; i++) {
			int vertex = workingSolution[i];
			if (workingMinusGuidingContains[vertex]) {
				if (adjacencyMatrix[swapInVertex][vertex] == 0) {
					tempWorkingSolution[tempWorkingSolutionSize] = vertex;
					tempWorkingSolutionSize++;
				}
				else {
					workingMinusGuidingContains[vertex] = false;
					int index = setDifferenceIndexMap[vertex];
					swap(workingMinusGuiding[index], workingMinusGuiding[workingMinusGuidingSize - 1]);
					setDifferenceIndexMap[workingMinusGuiding[index]] = index;
					workingMinusGuidingSize--;
				}
			}
			else {
				tempWorkingSolution[tempWorkingSolutionSize] = vertex;
				tempWorkingSolutionSize++;
			}
		}
		tempWorkingSolution[tempWorkingSolutionSize] = swapInVertex;
		tempWorkingSolutionSize++;

		guidingMinusWorkingContains[swapInVertex] = false;
		int index = setDifferenceIndexMap[swapInVertex];
		swap(guidingMinusWorking[index], guidingMinusWorking[guidingMinusWorkingSize - 1]);
		setDifferenceIndexMap[guidingMinusWorking[index]] = index;
		guidingMinusWorkingSize--;
		memset(workingContains, false, sizeof(bool) * numVertices);
		for (int i = 0; i < tempWorkingSolutionSize; i++) {
			workingSolution[i] = tempWorkingSolution[i];
			workingContains[tempWorkingSolution[i]] = true;
		}
		workingSolutionSize = tempWorkingSolutionSize;
	}


	void setCurrentToWorking() {
		resetLists();
		for (int i = 0; i < workingSolutionSize; i++) {
			int addIndex = indices[workingSolution[i]];
			addToCurrSolution(addIndex);
		}
		iterationCtr = 0;
	}

	void performPathRelinking(int runBest) {
		prStartTime = (double)clock();
		prFinishTime = prStartTime;
		prBestLength = runBestLength;
		prBestQuality = runBest;
		prIterationSolutionWasFoundOn = 0;

		int min = runBest;
		long iterationsUsed = 0;
		long iterationFound = 0;
		for (int i = 0; i < eliteSetSize - 1; i++) {
			memset(workingContains, false, sizeof(bool) * numVertices);
			for (int m = 0; m < eliteSetSizes[i]; m++) {
				initialSolution[m] = eliteSet[i][m];
				workingSolution[m] = eliteSet[i][m];
				workingContains[eliteSet[i][m]] = true;
			}
			workingSolutionSize = eliteSetSizes[i];
			initialSolutionSize = eliteSetSizes[i];
			for (int j = 0; j < eliteSetSize; j++) {
				if (j == i ) {
					continue;
				}
				memset(guidingContains, false, sizeof(bool) * numVertices);
				for (int m = 0; m < eliteSetSizes[j]; m++) {
					guidingSolution[m] = eliteSet[j][m];
					guidingContains[eliteSet[j][m]] = true;
				}
				guidingSolutionSize = eliteSetSizes[j];
				calculateDifferences();
				int steps = 0;
				while (guidingMinusWorkingSize > 0) {
					moveWorkingTowardsGuidingGreatestDelta();
					setCurrentToWorking();
					steps++;
					int currQual = currSolutionQuality;
					int afterPr = performLocalSearch(maxUnimproved, true);
					iterationsUsed += iterationCtr;
					if (afterPr > min) {
						min = afterPr;
						iterationFound = iterationsUsed;
						prFinishTime = (double)clock();
						prIterationSolutionWasFoundOn = iterationFound;
						prBestLength = currSolutionLength;
						prBestQuality = afterPr;
					}
				}
				if (steps == 0) {
					cout << "danger" << endl;
				}
			}
		}
		cout << "Before PR: " << runBest << " After: " << min << " Iterations: " << iterationFound << endl;
		eliteSetSize = 0;
		eliteSetMinWeight = INT32_MIN;
	}

	int runHeuristic()
	{
		int i, improvedSolutionQuality, runBest;
		runBest = 0;
		iterationSolutionWasFoundOn = 0;
		int totalIterations = 0;
		starting_time = (double)clock();
		for (i = 0; i < maxIterationsDividedByMaxUnimproved; i++)
		{
			improvedSolutionQuality = setInitialSolution();

			improvedSolutionQuality = performLocalSearch(maxUnimproved, false);
			
			if (localBestSolutionQuality >= eliteSetMinWeight) { //local best solution
				addLocalBestToEliteSet();
			}

			totalIterations = totalIterations + iterationCtr;
			if (improvedSolutionQuality > runBest)
			{
				runBest = improvedSolutionQuality;
				finishing_time = (double)clock();
				iterationSolutionWasFoundOn = totalIterations;
				runBestLength = localBestSolutionLength;
			}

			if (runBest >= bestKnownSolutionQuality)
				break;
		}
		performPathRelinking(runBest);
		return runBest;
	}

	int main(int argc, char **argv)
	{
		if (argc == 5)
		{
			File_Name = argv[1];
			bestKnownSolutionQuality = atoi(argv[2]);
			weightMod = atoi(argv[3]);
			maxUnimproved = atoi(argv[4]);
		}
		else
		{
			cout << "Error : the user should input four parameters to run the program." << endl;
			exit(0);
		}
		srand((unsigned)time(NULL));
		Initializing();
		numVerticesTimesSizeOfInt = numVertices * sizeof(int);
		cout << "finish reading data" << endl;
		int i, l;
		maxIterationsDividedByMaxUnimproved = (int(100000000 / maxUnimproved)) + 1;
		cout << "len_time = " << maxIterationsDividedByMaxUnimproved << endl;
		for (i = 0; i < 100; i++)
		{
			l = runHeuristic();
			solutionQuality[i] = l;
			len_used[i] = runBestLength;
			time_used[i] = finishing_time - starting_time;
			Iteration[i] = iterationSolutionWasFoundOn;

			prSolutionQuality[i] = prBestQuality;
			prLenUsed[i] = prBestLength;
			prTimeUsed[i] = prFinishTime - prStartTime;
			prIterations[i] = prIterationSolutionWasFoundOn;

			cout << "i = " << i << " l = " << l << endl;
		}

		printResultsToFile();
		cout << "finished" << endl;
		getchar();
	}