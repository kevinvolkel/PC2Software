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
#include"Winsock_Functions.h"
#include "SendFilestruct.h"
#pragma comment(lib,"ws2_32.lib") //Winsock Library



//function for initiating serial port
void setserial(HANDLE *serial_handle, DCB *serialparams);


//function for sending the EGM data to the mbed
void senddata(HANDLE *serial_handle, char *AS_top, char *AS_bot, char *VS_top, char *VS_bot, char *SS_top, char  *SS_bot,
	char **AS_top_p, char **AS_bot_p, char **VS_top_p, char **VS_bot_p, char **SS_top_p, char  **SS_bot_p);


//function for 'Go' Sequence for the mbed
void startmbed(HANDLE *serial_handle, int loop_num);


//funciton for converting text file data
void convert2_DAC(char *AStop, char *ASbot, char *VStop, char *VSbot, char *SStop, char *SSbot, double *buf_data_X, double *buf_data_Y, double *buf_data_Z, double LSB);

//function for receiving ADC data from mbed
void Receive_ADC_Data(HANDLE *serial_handle, float *ADC_voltage_data);

//function for making log file
void make_log(char send_num, double *buf_data_X, double *buf_data_Y, double *buf_data_Z, float *buf_ADC_voltage, std::string textfilename);

//function for sending the EGM file twice, and making a log file
void sendEGMfile(HANDLE *socket_handle, char *pointAStop, char *pointASbot, char* pointVStop, char *pointVSbot, char *pointSStop, char *pointSSbot,
	char *sendAStop, char *sendASbot, char *sendVStop, char *sendVSbot, char *sendSStop, char *sendSSbot, double *buffer_data_in_X, double *buffer_data_in_Y,
	double *buffer_data_in_Z, std::string textfilename, SOCKET *socket, struct Filestruct *struct_4log_send);