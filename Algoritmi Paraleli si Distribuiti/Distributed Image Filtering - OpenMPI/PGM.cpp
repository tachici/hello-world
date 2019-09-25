
#include "PGM.h"


PGM::PGM(const char * file_name) {
	strcpy(this->file_name, file_name);

	//open file:
	FILE * fp = fopen(file_name, "r");
	if (fp == NULL) {
		char err[100];
		sprintf(err, "Cant open file: %s.", file_name);
		perror (err);

		return;
	}

	 

	//load file in buffer:
	buffer = std::vector<char>(0);
	char c = 0;
	do {
		c = fgetc(fp);
		buffer.push_back(c);
	} while (c != EOF);

	/*
		parse the file:
		1. A "magic number" for identifying the file type. A pgm image's magic number is the two characters "P5".
		2. Whitespace (blanks, TABs, CRs, LFs).
		3. A width, formatted as ASCII characters in decimal.
		4. Whitespace.
		5. A height, again in ASCII decimal.
		6. Whitespace.
		7. The maximum gray value (Maxval), again in ASCII decimal. Must be less than 65536, and more than zero.
		8. A single whitespace character (usually a newline).
		9. pixelValue = whiteSpace + pixelValue + whiteSpace + ... whiteSpace + pixelValue + EOF
	*/

	int p = 0;

	parseWS(buffer, p);

	//parse magicNumber
	this->magicNumber[0] = buffer[p++];
	this->magicNumber[1] = buffer[p++];

	//parse white spaces
	parseWS(buffer, p);

	//parse width
	this->width = parseInteger(buffer, p);
    
	//parse white spaces
	parseWS(buffer, p);

	//parse height
	this->height = parseInteger(buffer, p);

	//parse white spaces
	parseWS(buffer, p);

	//parse max value:
	this->maxVal = parseInteger(buffer, p);


	//mark the start of pixels in file:
	pixelValuesStartPos = p;


	//create a pixel map:
	pixelMap = std::vector< std::vector<char> >(height + 2);
	for (int i = 0; i < height + 2; i++) {
		pixelMap[i] = std::vector<char>(width + 2);
	}

	//border with zeros lines:
	for (int i = 0; i < width + 2; i++) {
		pixelMap[0][i] = 0;
		pixelMap[height + 1][i] = 0;
	}

	//border with zeros columns:
	for (int i = 0; i < height + 2; i++) {
		pixelMap[i][0] = 0;
		pixelMap[i][width + 1] = 0;
	}

	//start parsing pixels:
	char pixelValue;
	int pixelNr = 0;
	for (int i = 1; i <= height; i++) {
		for (int j = 1; j <= width; j++) {
			pixelValue = (char)parseInteger(buffer, p);
			pixelMap[i][j] = pixelValue;
			parseWS(buffer, p);
		}
	}

	fclose(fp);

 
}

PGM::~PGM() {

}

void PGM::parseWS(const std::vector<char> &v, int &p) {
	while (1) {

		//parse wide space:
		while(isspace(v[p])) p++;

		//parse comment until newline:
		if (v[p] == '#') {
			
			while(v[p] != '\n') p++;
		}

		//parse wide space:
		while(isspace(v[p])) p++;
		
		//check if no longer a #:
		if (v[p] != '#') break;
	}
}

int PGM::parseInteger(const std::vector<char> &v, int &p) {
	int readInteger;

	char c = 0;
	char i = 0;
	char stringInt[10] = {0};
	do {
		c = v[p++];
		stringInt[i++] = c;
	} while (!isspace(c));
 
	return atoi(stringInt);
}

/*
	Write to the new file the header from the opened file.
	Write next the new_PixelMap in file.
*/
void PGM::writeModImage(char * newimageName, char ** new_PixelMap) {
	FILE * fp = fopen(newimageName, "w");

	//write header back to file:
	for (int i = 0; i < pixelValuesStartPos; i++) {
		fprintf(fp, "%c", buffer[i]);
	}


	//write all pixels:
	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {

			fprintf(fp, "%d\n", (unsigned char)new_PixelMap[i][j]);
		}	
	}

	fclose(fp);
}