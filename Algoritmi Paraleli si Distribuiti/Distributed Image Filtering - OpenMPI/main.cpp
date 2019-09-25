#include <iostream>
#include <mpi.h>
#include "topology.h"

using namespace std;
/*
	argv[1] - topologie.in
	argv[2] - imagini.in
	argv[3] - statistica.out
*/

int main(int argnr, char ** argv) {
	/*
		1. read + create topology
		2. for every image:
			- parse pgm image
			- process image
			- save new image
	*/
	MPI_Init(&argnr, &argv);
	int rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	Topology t = Topology(argv[1]);

	//determine if current process is a leaf / Intermediary / root:

	if (rank == 0) {
		Root r = Root(argv[1]);
		r.run(argv[2], argv[3]);
	} else  if (t.tree[rank].size() > 1) {
		//this node has a parent and a child => Intermediary
		//splits data forward:
		Intermediary intermediary = Intermediary(argv[1]);
		intermediary.run();
	} else if (t.tree[rank].size() == 1) {
		//has only one child => Leaf (Worker):
		Leaf leaf = Leaf();
		leaf.run();
	}
	
	

	return 0;
}