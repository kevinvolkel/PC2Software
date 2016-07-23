#pragma once
#include<string>
#include<iostream>
#include <math.h>
#include <sstream>
//structure for socket communication and file sending
struct Filestruct{
	std::string line3, masterfilename, textfilename, line, line2, masterfile, textfile;
	std::ifstream inFile;
	int linecount, counter, delimind, count;
	char buffer[160000];
	char *message, *bufpoint;
	char filenamebuf[200];
	int receivesize;
};