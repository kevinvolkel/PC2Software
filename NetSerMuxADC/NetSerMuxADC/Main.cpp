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

WSADATA wsa;
SOCKET sock, new_socket;
struct sockaddr_in server, client;







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
					ADCvoltage[ADC_in] = (ADCdata * 3.3) / (pow(2, 8));
				
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
			

				//send the log file 
				send_log_sock(&new_socket,&struct4_send, "data.txt");

				//close the datalog
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








