#include<io.h>
#include<stdio.h>
#include<winsock2.h>
#include <iostream>
#include <string>
#include <cmath>
#include <math.h>
#include <sstream>
#include <fstream>
#include <windows.h>
#include <conio.h>
#include"Winsock_Functions.h"
#include "SendFilestruct.h"

#pragma comment(lib,"ws2_32.lib") //Winsock Library

// function to initiate server
char initserver(WSADATA *wsa, SOCKET *sock, SOCKET *new_socket, sockaddr_in *server, sockaddr_in *client) {
	int c = 0;

	if (WSAStartup(MAKEWORD(2, 2), wsa) != 0)
	{
		printf("Failed. Error Code : %d", WSAGetLastError());
		return 1;
	}


	//Create a socket
	if ((*sock = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
	{
		printf("Could not create socket : %d", WSAGetLastError());
	}


	//Prepare the sockaddr_in structure
	server->sin_family = AF_INET;
	server->sin_addr.s_addr = INADDR_ANY;
	server->sin_port = htons(8888);

	//Bind
	if (bind(*sock, (struct sockaddr *)server, sizeof(*server)) == SOCKET_ERROR)
	{
		printf("Bind failed with error code : %d", WSAGetLastError());
	}




	//Listen to incoming connections
	listen(*sock, 3);

	//Accept and incoming connection


	c = sizeof(struct sockaddr_in);
	*new_socket = accept(*sock, (struct sockaddr *)client, &c);
	if (*new_socket == INVALID_SOCKET)
	{
		printf("accept failed with error code : %d", WSAGetLastError());
	}
}

//function to initiate the cliet, PC1
char initclient(WSADATA *wsa, SOCKET *sock, sockaddr_in *server) {

	//Initialize Winsock
	if (WSAStartup(MAKEWORD(2, 2), wsa) != 0)

	{

		printf("Failed. Error Code : %d", WSAGetLastError());

		return 1;

	}





	//Create a socket

	if ((*sock = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)

	{

		printf("Could not create socket : %d", WSAGetLastError());

	}





	//setsockopt(s, IPPROTO_TCP, TCP_NODELAY, 0, 1);

	//Remote server's IP address
	server->sin_addr.s_addr = inet_addr(/*"158.130.109.243"*/ "127.0.0.1");
	//Set adress family to IP version 4
	server->sin_family = AF_INET;
	//Set port number
	server->sin_port = htons(8888);

	//Connect to remote server

	if (connect(*sock, (struct sockaddr *)server, sizeof(*server)) < 0)

	{

		puts("connect error");

		return 1;

	}

	std::cout << "Connection made" << std::endl;

}


//funciton to send log files
void send_log_sock(SOCKET *new_socket,struct Filestruct *struct_log, std::string filename) {
	std::ifstream filesend(filename);

	struct_log->counter = 0;
	struct_log->bufpoint = struct_log->buffer;
	for (struct_log->line2; std::getline(filesend, struct_log->line2);) {



		//count the line
		struct_log->counter++;

		struct_log->line2.copy(struct_log->bufpoint, struct_log->line2.length(), 0);

		struct_log->delimind = static_cast<int>(struct_log->line2.length());

		//deliminate the line

		struct_log->bufpoint[struct_log->delimind] = 0;

		//new pointer, delimind+1

		struct_log->bufpoint = struct_log->bufpoint + (struct_log->delimind + 1);

		if ((struct_log->bufpoint - struct_log->buffer) >= 147000 || struct_log->counter == 30002) {

			send(*new_socket, struct_log->buffer, 148000, 0);

			struct_log->bufpoint = struct_log->buffer;

			//handshake

			struct_log->receivesize = recv(*new_socket, struct_log->buffer, 1000, 0);

			//if end of file, say so

			if (struct_log->counter == 30002) {

				struct_log->message = "This program is done sending";

				send(*new_socket, struct_log->message, strlen(struct_log->message), 0);

			}

			else {

				struct_log->message = "c";

				send(*new_socket, struct_log->message, strlen(struct_log->message), 0);

			}

		}

	}
	//reset variables, close ifstream
	struct_log->bufpoint = struct_log->buffer;
	struct_log->counter = 0;
	filesend.close();

}

//function for receiving EGM files from PC1
void rec_EGM_txt(SOCKET *new_socket,char *incoming,double *buf_data_N, double *buf_data_X, double *buf_data_Y, double *buf_data_Z) {
	int number_of_lines = 0;
	float nl, xl, yl, zl;
	char *message;
	int recind = 0;
	int receivesize;
	message = "ok";
	//code for receieving the text file from other pc
	while (1) {
		//receive main data
		receivesize = recv(*new_socket, incoming, 147000, 0);

		while (recind < receivesize) {
			//convert buffer into string
			std::string str(&incoming[recind]);
			//cout<<str<<endl;
			std::istringstream in(str);      //make a stream for the line itself
			in >> xl >> yl >> zl;
			buf_data_N[number_of_lines] = (double)number_of_lines;
			buf_data_X[number_of_lines] = xl;  //  * 0.02
			buf_data_Y[number_of_lines] = yl;  //  * 0.04
			buf_data_Z[number_of_lines] = zl;  //  * 0.01
			number_of_lines++;
			nl = 0;
			xl = 0;
			yl = 0;
			zl = 0;
			while (incoming[recind] != 0) recind++;
			recind++;

		}
		recind = 0;
		//handshake
		send(*new_socket, message, strlen(message), 0);
		//receive message on file status
		receivesize = recv(*new_socket, incoming, 1000, 0);
		incoming[receivesize] = 0;
		std::string str(incoming);
		if (str == "This program is done sending") break;
	}
} 

	//function to send EGM files from PC1 to PC2
void send_EGM_txt(SOCKET *s, struct Filestruct *struct4send){
	struct4send->linecount = 0;

	std::ifstream checkfile;

	std::ifstream inFiletext;

	inFiletext.open(struct4send->textfilename.c_str());

	//check to see if text file was opened
	if (!inFiletext) {

		std::cout << "Unable to open file: " << struct4send->textfilename << std::endl;


	}

	std::cout << "sending " << struct4send->line << std::endl;

	//loop to count the lines in the text file

	checkfile.open(struct4send->textfilename.c_str());

	for (struct4send->line3; std::getline(checkfile, struct4send->line3);) {

		struct4send->linecount++;

	}

	//send start message

	struct4send->message = "start";

	send(*s, struct4send->message, strlen(struct4send->message), 0);

	strcpy(struct4send->filenamebuf, struct4send->textfilename.c_str());

	//receive confirmation then send file name
	struct4send->receivesize = recv(*s, struct4send->buffer, 1000, 0);

	//send the name of the file
	send(*s, struct4send->filenamebuf, strlen(struct4send->filenamebuf), 0);

	//send lines from file

	for (struct4send->line2; std::getline(inFiletext, struct4send->line2);) {



		struct4send->counter++;

		struct4send->line2.copy(struct4send->bufpoint, struct4send->line2.length(), 0);

		struct4send->delimind = static_cast<int>(struct4send->line2.length());

		//deliminate the line

		struct4send->bufpoint[struct4send->delimind] = 0;

		//new pointer

		struct4send->bufpoint = struct4send->bufpoint + (struct4send->delimind + 1);

		if ((struct4send->bufpoint - struct4send->buffer) >= 147000 || struct4send->counter == struct4send->linecount) {

			send(*s, struct4send->buffer, 147000, 0);

			struct4send->bufpoint = struct4send->buffer;

			//handshake

			struct4send->receivesize = recv(*s, struct4send->buffer, 1000, 0);

			//if end of file, say so

			if (struct4send->counter == struct4send->linecount) {

				struct4send->message = "This program is done sending";

				send(*s, struct4send->message, strlen(struct4send->message), 0);

			}

			else {

				struct4send->message = "c";

				send(*s, struct4send->message, strlen(struct4send->message), 0);

			}

		}

	}
	//reset counters and close streams
	struct4send->counter = 0;

	inFiletext.close();

	checkfile.close();




}

int rec_log(SOCKET *s, int log_file_num, char *incoming, Filestruct *struct_rec_log) {
	int recind = 0;
	int receivesize = 0;
	std::string in_file_name;
	//receive two files, one for ICD one for Algorithm
	for (char rec_files = 1; rec_files < 3; rec_files++) {
		//create log file names
		if (rec_files == 1) in_file_name = "log_file_ICD" + std::to_string(log_file_num) + ".txt";
		if (rec_files == 2) in_file_name = "log_file_Open_ICD" + std::to_string(log_file_num) + ".txt";
		//open output stream for received data, also create the file 
		std::ofstream file_receive(in_file_name);

		//receive log file from other computer 
		while (1) {
			//receive main data
			receivesize = recv(*s, incoming, 148000, 0);

			while (recind < receivesize) {
				//convert buffer into string
				std::string str(&incoming[recind]);

				//stream string into the output file
				file_receive << str << std::endl;
				while (incoming[recind] != 0) recind++;
				recind++;
			}
			recind = 0;
			//handshake
			send(*s, struct_rec_log->message, strlen(struct_rec_log->message), 0);
			//receive message on file status
			receivesize = recv(*s, incoming, 1000, 0);
			incoming[receivesize] = 0;
			std::string str(incoming);
			if (str == "This program is done sending") break;
		}
		//close output stream so a new file can be imported 
		file_receive.close();
	}

	log_file_num++;
	return log_file_num;
}