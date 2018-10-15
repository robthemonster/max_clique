#pragma once

void Initializing();

void resetLists();

int selectNextVertexForInitialSolution();

int WSelectBestFromAdd();

int addToCurrSolution(int SelN);

int WSelectBestFromSwappy();

int doTheSwappy(int SelN);

int findMinWeightInCurrSolution();

int removeFromSolution();

int performLocalSearch(int maxUnimproved);

void printResultsToFile();

int runHeuristic();
