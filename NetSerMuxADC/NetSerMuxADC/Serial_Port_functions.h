#include <iostream>
#include <string>
#include <windows.h>

//function for initiating serial port
void setserial(HANDLE *serial_handle, DCB *serialparams);
//function for sending the EGM data to the mbed
void senddata(HANDLE *serial_handle, char *AS_top, char *AS_bot, char *VS_top, char *VS_bot, char *SS_top, char  *SS_bot,
	char **AS_top_p, char **AS_bot_p, char **VS_top_p, char **VS_bot_p, char **SS_top_p, char  **SS_bot_p);
//function for 'Go' Sequence for the mbed
void startmbed(HANDLE *serial_handle, int loop_num);
//funciton for converting text file data
void convert2_DAC(char *AStop, char *ASbot, char *VStop, char *VSbot, char *SStop, char *SSbot, double *buf_data_X, double *buf_data_Y, double *buf_data_Z, double LSB);