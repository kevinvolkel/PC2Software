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
#include "SendFilestruct.h"
#include"Serial_Port_functions.h"
#include"Winsock_Functions.h"



#pragma comment(lib,"ws2_32.lib") //Winsock Library

using namespace std;






// buffers for data from file

double buffer_data_in_N[30000];
double buffer_data_in_X[30000];
double buffer_data_in_Y[30000];
double buffer_data_in_Z[30000];
double buffer_data_OUT[150000];


//data buffers for sending data
char sendAStop[30000];
char *pointAStop = sendAStop;
char sendASbot[30000];
char *pointASbot = sendASbot;
char sendVStop[30000];
char *pointVStop = sendVStop;
char sendVSbot[30000];
char *pointVSbot = sendVSbot;
char sendSStop[30000];
char *pointSStop = sendSStop;
char sendSSbot[30000];
char *pointSSbot = sendSSbot;
char rec = 0;
int s = 0;
int count = 0;
// serial handle and variabels
HANDLE serialh;
// Set params
DCB serialparams = { 0 };
int error;
DWORD x, y;

//variables and structures for client server communication
char abortflag = 0;
char *message;
char incoming[200000];
int receivesize;

WSADATA wsa;
SOCKET sock, new_socket;
struct sockaddr_in server, client;







//variables for socket sending
struct Filestruct struct4_send;






int main() {
	
	ofstream ADC("ADCdata.txt");
	//LSB variable
	double LSB = 3.723e-5;

	message = "ok";
	// call to set up serial
	setserial(&serialh, &serialparams);
	CloseHandle(serialh);
	setserial(&serialh, &serialparams);


	//initiate server
	abortflag = initserver(&wsa, &sock, &new_socket, &server, &client);
	if (abortflag == 1) return 0;





	while (1) {
		cout << "Waiting for start" << endl << endl;
		//recieve initiation of data
		receivesize = recv(new_socket, incoming, 1000, 0);
		incoming[receivesize] = '\0';
		string init(incoming);
		if (init == "start") {
			send(new_socket, message, strlen(message), 0);
			receivesize = recv(new_socket, incoming, 1000, 0);
			incoming[receivesize] = '\0';
			string textfilename(incoming);
			cout << "Begin collecting Data" << endl << endl;

			//function for receiving EGM file from PC1
			rec_EGM_txt(&new_socket, incoming, buffer_data_in_N, buffer_data_in_X, buffer_data_in_Y, buffer_data_in_Z);

			cout << "data collected" << endl << endl;

			//funciton call that calculates values for the DAC
			convert2_DAC(sendAStop, sendASbot, sendVStop, sendVSbot, sendSStop, sendSSbot, buffer_data_in_X, buffer_data_in_Y, buffer_data_in_Z, LSB);

			//function that sends EGM data once to the ICD and once to the Algorithm, also gets ICD shock data from the microcontroller and makes and sends log files 
			sendEGMfile(&serialh, pointAStop, pointASbot, pointVStop, pointVSbot, pointSStop, pointSSbot,
				sendAStop, sendASbot, sendVStop, sendVSbot, sendSStop, sendSSbot, buffer_data_in_X, buffer_data_in_Y,
				buffer_data_in_Z, textfilename, &new_socket, &struct4_send);

			cout << "Finished " << textfilename << endl << endl;

			// wait for the ICD algorithm to end, send message so another file can come in
			receivesize = recv(new_socket, incoming, 1000, 0);
			send(new_socket, message, strlen(message), 0);

		}
		else if (init == "done") break;


	}
	
	//close serial port
	CloseHandle(serialh);
	return 0;
}








