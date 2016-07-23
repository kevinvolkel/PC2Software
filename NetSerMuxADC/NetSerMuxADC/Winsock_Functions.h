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
#pragma comment(lib,"ws2_32.lib") //Winsock Library

//initiate ther server
char initserver(WSADATA *wsa, SOCKET *sock, SOCKET *new_socket, sockaddr_in *server, sockaddr_in *client);

//function for sending log file from PC2
void send_log_sock(SOCKET *new_socket, struct Filestruct *struct_log, std::string filename);

//function for receiving txt file from PC1
void rec_EGM_txt(SOCKET *new_socket, char *incoming, double *buf_data_N, double *buf_data_X, double *buf_data_Y, double *buf_data_Z);