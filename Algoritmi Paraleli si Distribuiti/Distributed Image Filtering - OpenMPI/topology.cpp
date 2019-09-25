#include "topology.h"


Topology::Topology() {

}

Topology::Topology(const char * topology_file) {
	/*
		Parse topology file.
	*/
	FILE * fp = fopen(topology_file, "r");
	if (fp == NULL) {
		perror("Can't open topology_file\n");
	}
 
	char line[100];
	char * pch;

	tree = std::vector<std::vector<int> >(0);

	while (!feof(fp)) {
		//read a node children
		fgets(line, 100, fp);


		//split line by tokens:
		pch = strtok (line," :\n");

		//add neighbours for a node:
		tree.push_back(std::vector<int>(0));
		int currentNode = atoi(pch);


		while (pch != NULL)
		{
			pch = strtok (NULL, " :\n");
			if (pch == NULL) break;

			tree[currentNode].push_back(atoi(pch));
		}
	}

	fclose(fp);
}


void sendMessageToChildren(int messageTag, std::vector<int>& children) {
	int x = messageTag;
	for (int i = 0; i < children.size(); i++) {
		MPI_Send(&x, 1, MPI_INT, children[i], messageTag, MPI_COMM_WORLD);
	}
}

/*
	Receive the computed lines from all the children, knowing
	who was tasked with which lines:
*/
void receiveResultLines(char ** pixelMap, std::vector<MessageToChild>& messages, int line_length) {
	MPI_Status status;
	int rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	for (int i = 0; i < messages.size(); i++ ){
		//wait for results from every child:

		int child = messages[i].rank_child;
		int nr_lines_to_expect = messages[i].end - messages[i].start + 1;

		for (int j = 0; j < nr_lines_to_expect; j++) {
			
			MPI_Recv(pixelMap[messages[i].start + j - 1], line_length, MPI_CHAR, child, MPI_ANY_TAG, MPI_COMM_WORLD, &status);	
		}
	}

}

void receiveStatistics(std::vector<Statistic>& statistics ,std::vector<MessageToChild>& messages) {
	// statistic will be an array of ints:
	// (rank,nrLines), (rank,nrLines)...
	//MPI_Probe(int source, int tag, MPI_Comm comm, MPI_Status* status);

	MPI_Status status;

	//collect statistics from every child:
	for (int i = 0; i < messages.size(); i++) {
		MPI_Probe(messages[i].rank_child, STATISTICS, MPI_COMM_WORLD, &status);
		int number_amount = 0;

		//find out about the size of the message:
		MPI_Get_count(&status, MPI_INT, &number_amount);
		int nrStatistics = number_amount / 2;
		int * buffer = (int *)calloc(number_amount, sizeof(int));

		//receive statistics array
		MPI_Recv(buffer, number_amount, MPI_INT, messages[i].rank_child, STATISTICS, MPI_COMM_WORLD, &status);	

		for (int j = 0; j < number_amount; j = j + 2) {
			statistics.push_back(Statistic(buffer[j], buffer[j + 1]));
		}

		free(buffer);
	}

}

void sendStatistics(int to, const std::vector<Statistic>& statistics) {
	int * buffer = (int *)calloc(statistics.size() * 2, sizeof(int));

	//put statistics in buffer:
	for (int i = 0; i < statistics.size(); i++) {
		buffer[2 * i] = statistics[i].rank;
		buffer[2 * i + 1] = statistics[i].nrProcessedLines;
	}

	//send buffer:
	MPI_Ssend(buffer, statistics.size() * 2, MPI_INT, to, STATISTICS, MPI_COMM_WORLD);				

	free(buffer);
}

void printStatisticsToFile(const char * fileName, std::vector<Statistic>& statistics) {
	//sort by rank ascending:
	Statistic aux;
	for (int i = 0; i < statistics.size(); i++) {
		for (int j = i; j < statistics.size(); j++) {
			if (statistics[i].rank > statistics[j].rank) {
				aux = statistics[i];
				statistics[i] = statistics[j];
				statistics[j] = aux;
			}
		}
	}

	//print to file:
	FILE * fp = fopen(fileName, "w");
	fprintf(fp, "0: %d\n", 0);
	for (int i = 0; i < statistics.size(); i++) {
		fprintf(fp, "%d: %d\n", statistics[i].rank, statistics[i].nrProcessedLines);
	}

	fclose(fp);
}

void Root::run(const char * images_file, const char * statistics_file) {
	//Root has rank = 0
	int rank;
	int nProcesses;

	
	MPI_Status status;
	MPI_Request request;

	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &nProcesses);

	//parse imagini.in:
	FILE * fp = fopen(images_file, "r");
	if (fp == NULL) {
		perror("Cant open image file.");
		return;
	}

	//get number of entries:
	char commands_nr_string[10];
	fgets(commands_nr_string, 10, fp);
	int commands_nr = atoi(commands_nr_string);
	
	//remember lines sent to every child:
	std::vector<MessageToChild> messages = std::vector<MessageToChild>(0);
	
	char line[1000];
	for (int i = 0; i < commands_nr; i++) {
		messages.clear();

		//for every command:
		fgets(line, 1000, fp);

		//split line by tokens:
		char * p = strtok(line," \n");
		char command[300];
		strcpy(command, p);

		int currentCommand = 0;
		if (strcmp(command, "sobel") == 0) {
			//sobel command:
			currentCommand = SOBEL;
		} else if (strcmp(command, "mean_removal") == 0) {
			//mean_removal command:
			currentCommand = MEAN_REMOVAL;
		}


		p = strtok(NULL," \n");
		char image[300];
		strcpy(image, p);

		p = strtok(NULL," \n");
		char new_image[300];
		strcpy(new_image, p);

		//read image:
		PGM pgm = PGM(image);

		//send/receive data to/from every child:
		int bordedImgHeight = pgm.height + 2;

		int nrLinesPerChild = pgm.height / topology.tree[0].size();
		int start = 1;
		int end = nrLinesPerChild + 1;
		int line_size = pgm.width + 2;
		
		
		
		//send lines to children:
		//for every child in the list
		for (int j = 0; j < topology.tree[0].size(); j++) {
			//last child gets the last lines:
			
			if (j == topology.tree[0].size() - 1) {
				end = pgm.height + 1;
			}

			int linesToBeSent = end - start + 2; // 2 lines will be necesary for 3X3 multiplication.

			//send width of a line + borders:
			MPI_Send(&line_size, 1, MPI_INT, topology.tree[0][j], LINE_SIZE_T, MPI_COMM_WORLD);

			//send number of lines:
			MPI_Send(&linesToBeSent, 1, MPI_INT, topology.tree[0][j], NR_LINES_T, MPI_COMM_WORLD);


			//Send all lines + borders:
			//start line and end line will act as borders
			int buffer[line_size];
			int nrSentLines = 0;
			for (int k = start - 1; k <= end; k++) {


				//send a line:
				MPI_Send(pgm.pixelMap[k].data(), line_size, MPI_CHAR, topology.tree[0][j], currentCommand, MPI_COMM_WORLD);				
				nrSentLines++;
			}

			

			//will receive only the lines that are NOT borded by additional lines
			//good image lines start from 1 to height
			messages.push_back(MessageToChild(topology.tree[rank][j], start, end - 1));

			start += nrLinesPerChild;
			end += nrLinesPerChild;
		}

		
		//expect results:
		//create pixel map for storage:
		char ** pixelMap = (char **)calloc(sizeof(char*), pgm.height);

		//receive lines:
		for (int j = 0; j < pgm.height; j++)  {
			pixelMap[j] = (char *)calloc(sizeof(char), line_size);
		}

		//TODO: receive concatenation
		receiveResultLines(pixelMap, messages, line_size);

		pgm.writeModImage(new_image, pixelMap);

		//free mem:
		for (int j = 0; j < pgm.height; j++) {
			free(pixelMap[j]);
		}
		free(pixelMap);
	}

	//after processing every command send a termiantion tag:
	sendMessageToChildren(TERMINATION, topology.tree[rank]);

	//wait for statistics:
	std::vector<Statistic> statistics = std::vector<Statistic>(0);
	receiveStatistics(statistics, messages);

	//print statistics to file:
	printStatisticsToFile(statistics_file, statistics);


	fclose(fp);
	MPI_Finalize();

}



/*
	Get children ids from the topology file.
*/
Root::Root(const char * file) {
		topology = Topology(file);
}

Intermediary::Intermediary() { }

Intermediary::Intermediary(const char * file) {
	topology = Topology(file);
}

void Intermediary::run() {
	//has one parent
	//and multiple children
	int rank;
	int nProcesses;
 	int parent_id = 0;
 	int currentCommand = 0;

	MPI_Status status;
	MPI_Request request;

	int top[4][4];

	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &nProcesses);

	int width = 0;
	int height = 0;
	std::vector<MessageToChild> messages = std::vector<MessageToChild>(0);
	std::vector<Statistic> statistics = std::vector<Statistic>(0);

	while(1) {
		

		//receive size of a line:
		MPI_Recv(&width, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

		//remember parent rank:
		parent_id = status.MPI_SOURCE;
		
		//check if current node received a termination tag:
		if (status.MPI_TAG == TERMINATION) {
			//forward termination tag:
			sendMessageToChildren(TERMINATION, topology.tree[rank]);
			
			//await statistics:
			receiveStatistics(statistics, messages);

			//add itself to the statistics with 0 lines processed:
			statistics.push_back(Statistic(rank, 0));

			//forward upward statistics:
			sendStatistics(parent_id, statistics);

			break;
		}

		//delete parent from child list:
		std::vector<int>::iterator position = std::find(topology.tree[rank].begin(), topology.tree[rank].end(), parent_id);
		if (position != topology.tree[rank].end()) // == myVector.end() means the element was not found
	    	topology.tree[rank].erase(position);	

		//receive nr of lines:
		MPI_Recv(&height, 1, MPI_INT, MPI_ANY_SOURCE, NR_LINES_T, MPI_COMM_WORLD, &status);


		//create pixel map for storage:
		char ** pixelMap = (char **)calloc(sizeof(char*), height);

		//receive lines:
		for (int i = 0; i < height; i++)  {
			pixelMap[i] = (char *)calloc(sizeof(char), width);
			
			MPI_Recv(pixelMap[i], width, MPI_CHAR, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
		}
		currentCommand = status.MPI_TAG;

		//send and split work forward:
		int nrChildren = topology.tree[rank].size();	


		int start = 1;
		int nrLinesPerChild = (height - 2) / nrChildren;
		int end = nrLinesPerChild + 1;
		messages.clear();

		for (int i = 0; i < topology.tree[rank].size(); i++) {
			//last child gets the last lines:
			if (i == topology.tree[rank].size() - 1) {
				end = height - 1;
			}
			int linesToBeSent = end - start + 2;

			//send width of a line + borders:
			int line_size = width;
			MPI_Send(&line_size, 1, MPI_INT, topology.tree[rank][i], LINE_SIZE_T, MPI_COMM_WORLD);

			//send number of lines:
			MPI_Send(&linesToBeSent, 1, MPI_INT, topology.tree[rank][i], NR_LINES_T, MPI_COMM_WORLD);


			//Send all lines + borders:
			//start line and end line will act as borders
			int buffer[line_size];
			for (int j = start - 1; j <= end ; j++) {

				//send a line:
				MPI_Send(pixelMap[j], line_size, MPI_CHAR, topology.tree[rank][i], currentCommand, MPI_COMM_WORLD);				
			}

			//will receive only the lines that are NOT borded by additional lines
			//good image lines start from 1 to height
			messages.push_back(MessageToChild(topology.tree[rank][i], start, end - 1));


			start += nrLinesPerChild;
			end += nrLinesPerChild;
		}

		//: I. receive results:
		//: II. concatenate results
		receiveResultLines(pixelMap, messages, width);
		
		// III. Send results to parent.
		//send results to parent
		//without the bording lines:
		for (int j = 0; j < height - 2; j++) {
			//send a line:
			MPI_Ssend(pixelMap[j], width, MPI_CHAR, parent_id, currentCommand, MPI_COMM_WORLD);				
		}

		//free mem:
		for (int i = 0; i < height; i++) {
			free(pixelMap[i]);
		}
		free(pixelMap);
	}

	MPI_Finalize();
}

Leaf::Leaf() { }


/*
	(x,y) - upperLeft of 3*3.
*/
int convolute3x3(char ** m1, int m2[3][3], int x, int y) {
	int sum = 0;
	int upperLeftX = x - 1;
	int upperLeftY = y - 1;

	int newValue = 0;
	

	for (int i = 0; i < 3; i++) { //line m1
		for (int j = 0; j < 3; j++) {	//column m2
				
				int pixelValue = (unsigned char)m1[i + upperLeftX][j + upperLeftY];
				newValue = pixelValue * m2[i][j];
				sum = sum + newValue;
		}
	}

	return sum;
}

void Leaf::run() {
	//should have only one neighbour

	int rank;
	int nProcesses;

 
	MPI_Status status;
	MPI_Request request;

	int top[4][4];

	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &nProcesses);

	//receive:
	int width = 0;
	int height = 0;
	int parent_id = 0;
	int currentCommand = 0;
	int processedLines = 0;
	std::vector<Statistic> statistics = std::vector<Statistic>(0);

	while(1) {



		//receive size of a line:
		MPI_Recv(&width, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
		parent_id = status.MPI_SOURCE;
		
		if (status.MPI_TAG == TERMINATION) {

			//send statistics and terminate execution:
			statistics.push_back(Statistic(rank, processedLines));

			sendStatistics(parent_id, statistics);

			break;
		}

		//receive nr of lines:
		MPI_Recv(&height, 1, MPI_INT, MPI_ANY_SOURCE, NR_LINES_T, MPI_COMM_WORLD, &status);

		processedLines = processedLines + height - 2;


		//create pixel map for storage:
		char ** pixelMap = (char **)calloc(sizeof(char*), height);

		//create new pixel map:
		char ** newPixelMap = (char **)calloc(sizeof(char*), height);

		//receive lines:
		for (int i = 0; i < height; i++)  {
			pixelMap[i] = (char *)calloc(sizeof(char), width);
			newPixelMap[i] = (char *)calloc(sizeof(char), width);
			MPI_Recv(pixelMap[i], width, MPI_CHAR, parent_id, MPI_ANY_TAG, MPI_COMM_WORLD, &status);	
		}

		currentCommand = status.MPI_TAG;

		//compute results
		//SOBEL + MEAN_REMOVAL

		int sobel[3][3] = {{1, 0, -1}, {2, 0, -2}, {1, 0, -1}};
		int mean_removal[3][3] = {{-1, -1, -1}, {-1, 9, -1}, {-1, -1, -1}};

		int newValue = 0;
		for (int i = 1; i < height - 1; i++) {
			for (int j = 1; j < width - 1; j++) {

				if (currentCommand == MEAN_REMOVAL)
					newValue = convolute3x3(pixelMap, mean_removal, i, j);
				else if (currentCommand == SOBEL)
					newValue = convolute3x3(pixelMap, sobel, i, j) + 127;
				if (newValue < 0) {
					newValue = 0;
				} else if (newValue > 255) {
					newValue = 255;
				}

				newPixelMap[i][j] = newValue;
			}
		}

		//send results to parent
		//without the bording lines:
		int lines_sent = 0;
		for (int j = 1; j < height - 1; j++) {

			//send a line:
			lines_sent++;

			//send without the left 0 border: pixelMap[j] + 1
			MPI_Ssend(newPixelMap[j] + 1, width, MPI_CHAR, parent_id, currentCommand, MPI_COMM_WORLD);				
		}

		for (int i = 0; i < height; i++)  {
			free(pixelMap[i]); 
			free(newPixelMap[i]);
		}

		free(pixelMap);
		free(newPixelMap);
	}

	MPI_Finalize();

}

