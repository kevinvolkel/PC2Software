#include "Serial_Port_functions.h"
#include <iostream>
#include <string>
#include <windows.h>

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

