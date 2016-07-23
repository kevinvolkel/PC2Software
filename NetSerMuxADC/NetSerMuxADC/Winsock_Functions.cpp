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