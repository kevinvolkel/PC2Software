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
#include <iomanip>
#include"Serial_Port_functions.h"
#include"Winsock_Functions.h"
#include "SendFilestruct.h"
#pragma comment(lib,"ws2_32.lib") //Winsock Library

//set serial function
void setserial(HANDLE *serial_handle, DCB *serialparams) {
	//open Com3
	*serial_handle = CreateFile("\\\\.\\COM3", GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	//set various serial params 921600 bps, 1 stop bit
	serialparams->DCBlength = sizeof(*serialparams);
	GetCommState(*serial_handle, serialparams);
	serialparams->BaudRate = 921600;
	serialparams->ByteSize = 8;
	serialparams->StopBits = 1;
	serialparams->Parity = 0;
	SetCommState(*serial_handle, serialparams);
}


//send data function
void senddata(HANDLE *serial_handle, char *AS_top, char *AS_bot, char *VS_top, char *VS_bot, char *SS_top, char  *SS_bot
	,char **AS_top_p, char **AS_bot_p, char **VS_top_p, char **VS_bot_p, char **SS_top_p, char  **SS_bot_p) {
	int error;
	//write data byte by byte
	error = WriteFile(*serial_handle, AS_top, 1, NULL, NULL);

	for (int l = 0; l<10000; l++);
	error = WriteFile(*serial_handle, AS_bot, 1, NULL, NULL);
	for (int l = 0; l<10000; l++);

	error = WriteFile(*serial_handle, VS_top, 1, NULL, NULL);

	for (int l = 0; l<10000; l++);
	error = WriteFile(*serial_handle, VS_bot, 1, NULL, NULL);

	for (int l = 0; l<10000; l++);
	error = WriteFile(*serial_handle, SS_top, 1, NULL, NULL);

	for (int l = 0; l<10000; l++);
	error = WriteFile(*serial_handle, SS_bot, 1, NULL, NULL);
	for (int l = 0; l<10000; l++);
	
	//increment values of pointers used to send data
	(*AS_top_p)++;
	(*AS_bot_p)++;
	(*VS_top_p)++;
	(*VS_bot_p)++;
	(*SS_top_p)++;
	(*SS_bot_p)++;
	
}


//function to send intitial 'GO' sequence to mbed
void startmbed(HANDLE *serial_handle, int loop_num) {
	int error;
	//initiate characters, and mux values
	char g = 'G';
	char o = 'O';
	char muxICD = '0';
	char muxModel = '1';
	//send GO sequence with mux selector
	error = WriteFile(*serial_handle, &g, 1, NULL, NULL);
	for (int l = 0; l < 10000; l++);
	error = WriteFile(*serial_handle, &o, 1, NULL, NULL);
	for (int l = 0; l < 10000; l++);
	if (loop_num == 0)error = WriteFile(*serial_handle, &muxICD, 1, NULL, NULL);
	else error = WriteFile(*serial_handle, &muxModel, 1, NULL, NULL);
	for (int l = 0; l < 10000; l++);
}


//function to convert text file data to 16 bit number for the DAC
void convert2_DAC(char *AStop, char *ASbot, char *VStop, char *VSbot, char *SStop, char *SSbot, double *buf_data_X, double *buf_data_Y, double *buf_data_Z, double LSB) {

	for (int i = 0; i < 30000; i++) {

		AStop[i] = ((short)((buf_data_X[i] + 1.02) / (LSB))) >> 8;
		ASbot[i] = (short)((buf_data_X[i] + 1.02) / (LSB));
		VStop[i] = ((short)((buf_data_Y[i] + 1.02) / (LSB))) >> 8;
		VSbot[i] = (short)((buf_data_Y[i] + 1.02) / (LSB));
		SStop[i] = ((short)((buf_data_Z[i] + 1.02) / (LSB))) >> 8;
		SSbot[i] = (short)((buf_data_Z[i] + 1.02) / (LSB));
	}
}

//function for receiving ADC values from mbed
void Receive_ADC_Data(HANDLE *serial_handle,float *ADC_voltage_data) {
	char rec;
	unsigned char ADCdata;

	//fill buffer with voltage values, 30000 samples
	for (int ADC_in = 0; ADC_in < 30000; ADC_in++) {
		while (!ReadFile(*serial_handle, &rec, 1, NULL, NULL));
		ADCdata = rec;
		ADC_voltage_data[ADC_in] = (ADCdata * 3.3) / (pow(2, 8));

	}
}

//function for making log files
void make_log(char send_num,double *buf_data_X, double *buf_data_Y, double *buf_data_Z, float *buf_ADC_voltage, std::string textfilename) {
	
	std::ofstream datalog("data.txt");

	if (send_num == 0) datalog << textfilename << "   ICD Data" << std::endl << std::endl;
	else datalog << textfilename << "   ICD Algorithm Data" << std::endl << std::endl;
	datalog << "Atrial" << "\t\t" << "Ventricle" << "\t" << "Shock" << "\t" << "    ICD Shock"<<'\n';
	//fill up log file
	datalog << std::fixed;
	datalog << std::setprecision(6);
	for (int log_file_ind = 0; log_file_ind < 30000; log_file_ind++) {
		datalog << buf_data_X[log_file_ind] << "    \t" << buf_data_Y[log_file_ind] << "    \t" << buf_data_Z[log_file_ind] <<
			"    \t" << buf_ADC_voltage[log_file_ind] << '\n';
		
	}

	//close the datalog
	datalog.close();



}


//function for sending the EGM file twice send the data once for model and once for real ICD
void sendEGMfile(HANDLE *serial_handle, char *pointAStop, char *pointASbot, char* pointVStop, char *pointVSbot, char *pointSStop, char *pointSSbot,
	char *sendAStop, char *sendASbot, char *sendVStop, char *sendVSbot, char *sendSStop, char *sendSSbot, double *buffer_data_in_X, double *buffer_data_in_Y,
	double *buffer_data_in_Z, std::string textfilename, SOCKET *socket, struct Filestruct *struct_4log_send) {
	// buffer for aDC voltage data
	float ADCvoltage[30000];
	int error;
	char rec;
	for (int n = 0; n < 2; n++) {

		//tell mbed that data is coming
		startmbed(serial_handle, n);

		//loop to fill initial buffer of uC,start sending data
		for (int i = 0; i < 10; i++)
		{
			//send values and pointers to pointers so the pointers can be incremented
			senddata(serial_handle, pointAStop, pointASbot, pointVStop, pointVSbot, pointSStop, pointSSbot,
				&pointAStop, &pointASbot, &pointVStop, &pointVSbot, &pointSStop, &pointSSbot);

		}

		//enter loop for refilling buffer of microcontroller
		while (1) {
			//read serial port
			error = ReadFile(*serial_handle, &rec, 1, NULL, NULL);
			// if ! character was picked up send 5 bytes of data
			if (rec == '!') {
				rec = 0;
				for (int i = 0; i < 2; i++)
				{

					//send more values
					senddata(serial_handle, pointAStop, pointASbot, pointVStop, pointVSbot, pointSStop, pointSSbot,
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
				std::cout << "Shock Received" << std::endl;
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
		while (rec != 'D')error = ReadFile(*serial_handle, &rec, 1, NULL, NULL);
		//only make log for ICD device
		if (n == 0) {
			//Get Analog data from the microcontroller, after either DAQ orICD, 300000 samples, put into ADCvoltage buffer
			Receive_ADC_Data(serial_handle, ADCvoltage);

			//write a log text file with the acquired ADC values
			make_log(n, buffer_data_in_X, buffer_data_in_Y, buffer_data_in_Z, ADCvoltage, textfilename);


			//send the log file 
			send_log_sock(socket, struct_4log_send, "data.txt");
		}

	}
}
