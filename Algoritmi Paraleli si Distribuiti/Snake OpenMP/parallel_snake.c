#include "parallel_snake.h"


/*
	Check if two coordinates match.
	1 - if equal
	0 - if different
*/
int sameCoord(const struct coord * a, const struct coord * b) {
	return (a->line == b->line) && (a->col == b->col);
}

/*
	last: last snake segment visited
	current: curent snake segment

	returns: next snake segment if there is one
	else makes noSegment 1.
*/
struct coord getNextSegment(struct coord * last, struct coord * current, int * noSegment, int num_lines, int num_cols, int **world) {
	
	struct coord nextSegment;
	int snakeId = world[current->line][current->col];
	
	(*noSegment) = 0;

	//check nord:
	nextSegment.line = (current->line + 1) % num_lines;
	nextSegment.col = current->col;
	if (!sameCoord(&nextSegment, last) && world[nextSegment.line][nextSegment.col] == snakeId) {
		return nextSegment;
	}

	//check south:
	nextSegment.line = (current->line - 1 + num_lines) % num_lines;
	nextSegment.col = current->col;
	if (!sameCoord(&nextSegment, last) && world[nextSegment.line][nextSegment.col] == snakeId) {
		return nextSegment;
	}

	//check East:
	nextSegment.line = current->line;
	nextSegment.col = (current->col + 1) % num_cols;
	if (!sameCoord(&nextSegment, last) && world[nextSegment.line][nextSegment.col] == snakeId) {
		return nextSegment;
	}

	//check West:
	nextSegment.line = current->line;
	nextSegment.col = (current->col - 1 + num_cols) % num_cols;
	if (!sameCoord(&nextSegment, last) && world[nextSegment.line][nextSegment.col] == snakeId) {
		return nextSegment;
	}

	//no adequate next segment found:
	(*noSegment) = 1;
	return nextSegment;
}


/*
	s - a snake structure
	returns: coord structure containing the coordinates of snake's tail
*/
Snake * getSnake(struct snake * s, int num_lines, int num_cols, int **world) {

	//start from head	
	int reachedEnd = 0;
	Snake * snake = (Snake *)calloc(1, sizeof(Snake));
	snake->id = s->encoding;
	snake->length = 0;
	snake->dir = s->direction;
	//initial snake body capacity:
	snake->size = 2;
	snake->v = (struct coord *)calloc(10, sizeof(struct coord));

	//current snake segment:
	struct coord currentSeg = s->head;
	struct coord lastSeg = s->head;
	struct coord aux;

	//assemble Snake structure by identifying snake's body:
	while (reachedEnd == 0) {
		aux = currentSeg;

		//get the next piece of body of the snake
		currentSeg = getNextSegment(&lastSeg, &currentSeg, &reachedEnd, num_lines, num_cols, world);
		
		lastSeg = aux;
		//insert last found segment in the Snake strucutre:
		insertCoord(lastSeg, snake);

	}

	//return the snake tail line and col
	return snake;
}

void insertCoord(struct coord c, Snake * snake) {
	if (snake->size + 1 <= snake->length) {
		//double size:
		struct coord * auxV = (struct coord *)calloc(2 * snake->size, sizeof(struct coord *));

		//copy previous coordinates
		memcpy(auxV, snake->v, snake->length * sizeof(struct coord *));

		//free last coordinates:
		free(snake->v);

		//replace 
		snake->v = auxV;

		//insert the new coordinate:
		snake->v[snake->length] = c;

		//increase length
		snake->length++;

		//double capacity:
		snake->size = 2 * snake->size;
	} else {
		snake->v[snake->length] = c;
		snake->length++;
	}
}

struct coord getSnakeNextHead(Snake * x, int num_lines, int num_cols, int ** world) {
	struct coord nextPoz;
	switch(x->dir) {
			case 'N': {
				nextPoz.line = (x->v[0].line - 1 + num_lines) % num_lines;
				nextPoz.col = x->v[0].col;
				break;
			}

			case 'S': {
				nextPoz.line = (x->v[0].line + 1) % num_lines;
				nextPoz.col = x->v[0].col;
				break;
			}

			case 'E': {
				nextPoz.line = x->v[0].line;
				nextPoz.col = (x->v[0].col + 1) % num_cols;
				break;
			}

			case 'V': {
				nextPoz.line = x->v[0].line;
				nextPoz.col = (x->v[0].col - 1 + num_cols) % num_cols;
				break;
			}
	}
	return nextPoz;
}

void advance(Snake * x, struct coord newHead) {
	//shift right the snake's body:
	memmove(&(x->v[1]), &(x->v[0]), (x->length - 1) * sizeof(double));

	//x->v = memmove(x->v + sizeof(struct coord), x->v, sizeof(struct coord) * (x->size - 1));

	//insert on first position the new head:
	x->v[0].line = newHead.line;
	x->v[0].col = newHead.col;
}

void run_simulation(int num_lines, int num_cols, int **world, int num_snakes,
	struct snake *snakes, int step_count, char *file_name) 
{
	// TODO: Implement Parallel Snake simulation using the default (env. OMP_NUM_THREADS) 
	// number of threads.
	//
	// DO NOT include any I/O stuff here, but make sure that world and snakes
	// parameters are updated as required for the final state.
	
	//imported from env OMP
	int numberOfThreads = atoi(getenv ("OMP_NUM_THREADS"));
	omp_set_num_threads(numberOfThreads);

	int i = 0;
	int collisionFlag = 0;
	struct coord * nextHeadCoords = nextHeadCoords = (struct coord *)calloc(num_snakes, sizeof(struct coord));;

	Snake ** snakesStruct = (Snake **)calloc(num_snakes, sizeof(Snake *));

	#pragma omp parallel for private(i)
	for (i = 0; i < num_snakes; i++) {
		snakesStruct[i] = getSnake(&snakes[i], num_lines, num_cols, world);
	}

	while(step_count > 0) {

		//delete all tails:
		for (i = 0; i < num_snakes; i++) {
			Snake * s = snakesStruct[i];
			struct coord snakeTail = s->v[s->length - 1];
			world[snakeTail.line][snakeTail.col] = 0;
		}

		//determine all snake's next position (a snake must asume an unique new head position otherwise collision will occur):
		for (i = 0; i < num_snakes; i++) {
			nextHeadCoords[i] = getSnakeNextHead(snakesStruct[i], num_lines, num_cols, world);
		}

		//check every snake for collision:
		#pragma omp parallel for private(i)
		//for (i = 0; i < num_snakes && collisionFlag == 0; i++) {
		for (i = 0; i < num_snakes; i++) {
			if ( world[nextHeadCoords[i].line][nextHeadCoords[i].col] == 0) {
				//check if another snake will attack the same square:
				int j = 0;
				for (j = 0; j < num_snakes; j++) {
					if (sameCoord(&nextHeadCoords[i],&nextHeadCoords[j]) == 1 && i != j) {
						//two snakes will attack same square => collision
						collisionFlag = 1;
						//break;
					}
				}
			} else {
				//snake[i] will attack a marked square. Ex: 2, 3, etc.
				collisionFlag = 1;
			}
		}

		if (collisionFlag == 0) {
			//update snake positions (no collision will occur)
			//erase all tails:
			for (i = 0; i < num_snakes; i++) {
				Snake * snk = snakesStruct[i];
				struct coord snakeTail = snk->v[snk->length - 1];
				world[snakeTail.line][snakeTail.col] = 0;
			}

			//write new head positions:
			for (i = 0; i < num_snakes; i++) {
				Snake * snk = snakesStruct[i];
				world[nextHeadCoords[i].line][nextHeadCoords[i].col] = snakesStruct[i]->id;
			}


			//advance snake vectors:
			#pragma omp parallel for private(i)
			for (i = 0; i < num_snakes; i++) {
				advance(snakesStruct[i], nextHeadCoords[i]);
			}
		} else {
			//a collision occured:
 

			//reconstruct world map:
			//replace all tails:
			//delete all tails:
			for (i = 0; i < num_snakes; i++) {
				Snake * s = snakesStruct[i];
				struct coord snakeTail = s->v[s->length - 1];
				world[snakeTail.line][snakeTail.col] = s->id;
			}

			break;
		}

		//decrement step_count
		step_count--;
	} 

 	//update current snake heads:
		for (i = 0; i < num_snakes; i++) {
			snakes[i].head = snakesStruct[i]->v[0];
		}

	/*
		Snake * blyat = snakesStruct[2];
		for (i = 0; i < blyat->length; i++) {
			printf("(%d,%d)\n", blyat->v[i].line, blyat->v[i].col);
		}
		printf("Next: (%d,%d), on dir: %c\n", getSnakeNextHead(blyat, num_lines, num_cols, world).line, getSnakeNextHead(blyat, num_lines, num_cols, world).col, blyat->dir);
		printf("Tail: (%d, %d)\n", blyat->v[blyat->length - 1].line, blyat->v[blyat->length - 1].col);
	*/
	

	//free used mem:
	for (i = 0; i < num_snakes; i++) {
		free (snakesStruct[i]->v);
		free(snakesStruct[i]);
	}
	free(snakesStruct);

	free(nextHeadCoords);
}
