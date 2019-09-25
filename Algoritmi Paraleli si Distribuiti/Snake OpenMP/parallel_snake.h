#ifndef __PARALLEL_SNAKE_H__
#define __PARALLEL_SNAKE_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "main.h"
#include <omp.h>

typedef struct s {
	
	//v: [Head][body][body]...[body][tail]
	struct coord * v;
	
	int id;
	char dir;
	int length;	//number of snake segments
	int size;	//capacity of v
}Snake;

/*
	Shifts the internal array of Snake structure, and adds the new coordinate to the front.
	Propagates the results in world.

*/
void advance(Snake * x, struct coord newHead);

/*
	Creates a Snake structure by determining the begin and the end of the snake.
	Saves the body of the snake.
	Knows the length of it.
*/
Snake * getSnake(struct snake * s, int num_lines, int num_cols, int **world);


/*
	Inserts a new coord in array.
	Doubles the size of the array if needed.
	Returns the new size of array.
*/
void insertCoord(struct coord c, Snake * s);

/*
	Get snake future head coordiantes.
*/
struct coord getSnakeNextHead(Snake * x, int num_lines, int num_cols, int ** world);

int sameCoord(const struct coord * a, const struct coord * b);
struct coord getNextSegment(struct coord * last, struct coord * current, int * noSegment, int num_lines, int num_cols, int **world);
struct coord getTailCoord(struct snake * s, int num_lines, int num_cols, int **world);

#endif
