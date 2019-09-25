
#ifndef _TOPOLOGY_H_
#define _TOPOLOGY_H_

#include "PGM.h"

#define LINE_SIZE_T 3523
#define NR_LINES_T 3524
#define SOBEL 235
#define MEAN_REMOVAL 3621             
#define TERMINATION 666
#define STATISTICS 101

/*
	A topology consists of:
	Root:
		- reads imagini.in
		- for every entry in imagini.in:
			- reads the image file
			- parses image file
			- adds zero borders to the image matrix
			- splits commands in the tree (lines to be processed)
			- awaits responses (processed lines)
			- concatenates responses
			- sends statistical data requests
			- concatenates received data
			- writes in output file
			- reapeats above for every image file
	Intermediarys:
		- awaits command
		- splits command to children
		- awaits response
		- concatenates response
		- sends response to parent
		- awaits statistical data request
		- forwards request
		- concatenates responses
		- sends response to parent
	Leaves:
		- awaits command
		- processes command
		- sends response
		- awaits statistical command
		- sends statistical data
*/

/*
	Remember which child was tasked with what lines:
*/
class MessageToChild {

public:
	int rank_child;
	int start;
	int end;
	MessageToChild() {}
	MessageToChild(int rank_child, int start, int end) {
		this->rank_child = rank_child;
		this->start = start;
		this->end = end;
	}
	void print(int rank) {
		printf("INTERMEDIARY[%d]:[[From Intermediary[%d] to %d [%d,%d]]]\n",rank,  rank_child, rank, start, end);
	}
};

class Statistic {
public:
	int rank;
	int nrProcessedLines;

	Statistic() {

	}

	Statistic(int rank, int nrProcessedLines) {
		this->rank = rank;
		this->nrProcessedLines = nrProcessedLines;
	}
};

class Topology {
public:
	std::vector<std::vector<int> > tree;
	Topology(const char * topology_file);
	Topology();
};

class Root {
public:
	Topology topology;

	Root(const char * topology_file);

	void run(const char * images_file, const char * statistics_file);
};

class Intermediary {
public:
	Topology topology;
	Intermediary();
	Intermediary(const char * topology_file);
	void run();
};

class Leaf {
public:
    
	Leaf();
    
	void run();
};



#endif