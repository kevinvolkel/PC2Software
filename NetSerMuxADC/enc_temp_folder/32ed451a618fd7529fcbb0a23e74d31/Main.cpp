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



#pragma comment(lib,"ws2_32.lib") //Winsock Library

using namespace std;






// buffers for data from file
int number_of_lines = 0;
double buffer_data_in_N[30000];
double buffer_data_in_X[30000];
double buffer_data_in_Y[30000];
double buffer_data_in_Z[30000];
double buffer_data_OUT[150000];
float nl, xl, yl, zl;
// variables for the ADC recieving
unsigned char ADCdata;
float ADCvoltage[30000];
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
int recind = 0;
WSADATA wsa;
SOCKET sock, new_socket;
struct sockaddr_in server, client;
int c;



//initiate client
void initserver(void);

//variables for socket sending
struct Filestruct struct4_send;






int main() {
	ofstream datalog("data.txt");
	ofstream ADC("ADCdata.txt");
	//LSB variable
	double LSB = 3.723e-5;

	message = "ok";
	// call to set up serial
	setserial(&serialh, &serialparams);
	CloseHandle(serialh);
	setserial(&serialh, &serialparams);


	//initiate client
	initserver();
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


			//code for receieving the text file from other pc
			while (1) {
				//receive main data
				receivesize = recv(new_socket, incoming, 147000, 0);

				while (recind < receivesize) {
					//convert buffer into string
					string str(&incoming[recind]);
					//cout<<str<<endl;
					istringstream in(str);      //make a stream for the line itself
					in >> xl >> yl >> zl;
					buffer_data_in_N[number_of_lines] = (double)number_of_lines;
					buffer_data_in_X[number_of_lines] = xl;  //  * 0.02
					buffer_data_in_Y[number_of_lines] = yl;  //  * 0.04
					buffer_data_in_Z[number_of_lines] = zl;  //  * 0.01
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
				send(new_socket, message, strlen(message), 0);
				//receive message on file status
				receivesize = recv(new_socket, incoming, 1000, 0);
				incoming[receivesize] = 0;
				string str(incoming);
				if (str == "This program is done sending") break;

			}

			number_of_lines = 0;
			cout << "data collected" << endl << endl;

			//funciton call that calculates values for the DAC
			convert2_DAC(sendAStop, sendASbot, sendVStop, sendVSbot, sendSStop, sendSSbot, buffer_data_in_X, buffer_data_in_Y, buffer_data_in_Z, LSB);

			//send the data once for model and once for real ICD
			for (int n = 0; n < 2; n++) {
				ofstream datalog("data.txt");
				
				//start up th mbed
				startmbed(&serialh, n);



				//loop to fill initial buffer of uC,start sending data
				for (int i = 0; i < 10; i++)
				{
					//send values and pointers to pointers so the pointers can be incremented
					senddata(&serialh, pointAStop,pointASbot,pointVStop,pointVSbot,pointSStop,pointSSbot, 
						&pointAStop, &pointASbot, &pointVStop, &pointVSbot, &pointSStop, &pointSSbot);

				}

				//enter loop for refilling buffer of microcontroller
				while (1) {
					//read serial port
					error = ReadFile(serialh, &rec, 1, NULL, NULL);
					// if ! character was picked up send 5 bytes of data
					if (rec == '!') {
						rec = 0;
						for (int i = 0; i < 2; i++)
						{
							
							//send more values
							senddata(&serialh, pointAStop, pointASbot, pointVStop, pointVSbot, pointSStop, pointSSbot,
								&pointAStop, &pointASbot, &pointVStop, &pointVSbot, &pointSStop, &pointSSbot);


							//if the end of the data is reached, break out of loop
							if (pointAStop == (&sendAStop[30000])) {
								break;
							}
						}
					}
					//If char S is received report that a shock was sensed
					else if (rec == 'S') {
						rec = 0;
						cout << "Shock Received" << endl;
					}
					//break out of while loop at the end of the data
					if (pointAStop == &sendAStop[30000]) break;
				}
				// reset pointers to original values
				pointAStop = sendAStop;
				pointASbot = sendASbot;
				pointVStop = sendVStop;
				pointVSbot = sendVSbot;
				pointSStop = sendSStop;
				pointSSbot = sendSSbot;
				//wait for microcontroller to say when it sent all the data
				while (rec != 'D')error = ReadFile(serialh, &rec, 1, NULL, NULL);
				//Get Analog data from the microcontroller, after either DAQ orICD, 300000 samples
				for (int ADC_in = 0; ADC_in < 30000; ADC_in++) {
					while (!ReadFile(serialh, &rec, 1, NULL, NULL));
					ADCdata = rec;
					//calculate voltage value
					ADCvoltage[ADC_in] = (ADCdata * 3.3) / (pow(2, 8));
					//write voltage value to tex file 
					//ADC << ADCvoltage[ADC_in] << endl;
				}

				//write name of finsihed text file along with what data it is
				if (n == 0) datalog << textfilename << "   ICD Data" << endl << endl;
				else datalog << textfilename << "ICD Algorithm Data" << endl << endl;
				datalog << "Atrial" << "\t\t" << "Ventricle" << "\t" << "Shock" << "\t" << "    ICD Shock" << endl;
				//fill up log file
				for (int log_file_ind = 0; log_file_ind < 30000; log_file_ind++) {
					datalog << buffer_data_in_X[log_file_ind] << "    \t" << buffer_data_in_Y[log_file_ind] << "    \t" << buffer_data_in_Z[log_file_ind] <<
						"    \t" << ADCvoltage[log_file_ind] << endl;
				}
				//open data file for sending
				ifstream filesend("data.txt");
				struct4_send.counter = 0;
				struct4_send.bufpoint = struct4_send.buffer;

			

				//send data file
				for (struct4_send.line2; std::getline(filesend, struct4_send.line2);) {



					//count the line
					struct4_send.counter++;

					struct4_send.line2.copy(struct4_send.bufpoint, struct4_send.line2.length(), 0);

					struct4_send.delimind = static_cast<int>(struct4_send.line2.length());

					//deliminate the line

					struct4_send.bufpoint[struct4_send.delimind] = 0;

					//new pointer, delimind+1

					struct4_send.bufpoint = struct4_send.bufpoint + (struct4_send.delimind + 1);

					if ((struct4_send.bufpoint - struct4_send.buffer) >= 147000 || struct4_send.counter == 30002) {

						send(new_socket, struct4_send.buffer, 148000, 0);

						struct4_send.bufpoint = struct4_send.buffer;

						//handshake

						struct4_send.receivesize = recv(new_socket, struct4_send.buffer, 1000, 0);

						//if end of file, say so

						if (struct4_send.counter == 30002) {

							struct4_send.message = "This program is done sending";

							send(new_socket, struct4_send.message, strlen(struct4_send.message), 0);

						}

						else {

							struct4_send.message = "c";

							send(new_socket, struct4_send.message, strlen(struct4_send.message), 0);

						}

					}

				}
				//reset variables, close ifstream
				struct4_send.bufpoint = struct4_send.buffer;
				struct4_send.counter = 0;
				filesend.close();
				datalog.close();
			}
			
			cout << "Finished " << textfilename << endl << endl;

			//send message to say a new file can come
			send(new_socket, message, strlen(message), 0);

		}
		else if (init == "done") break;


	}
	
	//close serial port
	CloseHandle(serialh);
	return 0;
}









//function to initiate server
void initserver(void) {

	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		printf("Failed. Error Code : %d", WSAGetLastError());
		abortflag = 1;
	}


	//Create a socket
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
	{
		printf("Could not create socket : %d", WSAGetLastError());
	}


	//Prepare the sockaddr_in structure
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(8888);

	//Bind
	if (bind(sock, (struct sockaddr *)&server, sizeof(server)) == SOCKET_ERROR)
	{
		printf("Bind failed with error code : %d", WSAGetLastError());
	}




	//Listen to incoming connections
	listen(sock, 3);

	//Accept and incoming connection


	c = sizeof(struct sockaddr_in);
	new_socket = accept(sock, (struct sockaddr *)&client, &c);
	if (new_socket == INVALID_SOCKET)
	{
		printf("accept failed with error code : %d", WSAGetLastError());
	}
}