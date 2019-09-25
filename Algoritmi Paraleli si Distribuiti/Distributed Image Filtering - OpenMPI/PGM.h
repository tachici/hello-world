#ifndef _PGM_H_
#define _PGM_H_

#include <iostream>
#include <stdio.h>
#include <string.h>
#include <vector>
#include <ctype.h>
#include <stdlib.h>
#include <mpi.h>
#include <algorithm>

class PGM {
public:
	char file_name[1000];
	char magicNumber[2];
	int width;	//of image
	int height;
	int maxVal;
	int pixelValuesStartPos; //pixel values start position in file
	std::vector<char> buffer; //file is saved in a buffer of chars
	std::vector<char> pixelValues;	//pixels have a value from 0 to 255
	std::vector< std::vector<char> > pixelMap; //contains the image + zero borders
	/*
		Loads a .pgm image into a PGM class
	*/
	PGM(const char * file_name);
	~PGM();
	void parseWS(const std::vector<char> &v, int &p);
	int parseInteger(const std::vector<char> &v, int &p);
	void writeModImage(char * newimageName, char ** new_PixelMap);
};

#endif