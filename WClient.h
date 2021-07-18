#include<WINSOCK2.H>
#include<STDIO.H>
#include<iostream>
#include<cstring>
#include<sys/stat.h> 
#include<sys/types.h>
#include<direct.h>
#include<time.h>
#include<io.h>
#include<windows.h>
#pragma warning(disable:4996)
using namespace std;
#pragma comment(lib, "ws2_32.lib")


bool Writen(const int sockfd,const char *buffer,const size_t n);

bool Readn(const int sockfd,char *buffer,const size_t n);

bool TcpWrite(const int sockfd,const char *buffer,const int ibuflen=0);

bool TcpRead(const int sockfd,char *buffer,int *ibuflen,const int itimeout = 0);

bool SendFile(int sockfd,struct FileInfo *fileinfo);

FILE *FOPEN(const char *filename,const char *mode);

int FileSize(const char *filename);

bool MKDIR(const char *pathorfilename,bool bisfilename=true);

void timetostr(const time_t ltime,char *stime,const char *fmt=0);

bool FileMTime(const char *filename,char *mtime,const char *fmt=0);

void LocalTime(char *stime,const char *fmt=0,const int timetvl=0);

void showMenu();

string getName(const char* full_name);

class WinClient{
private:
	sockaddr_in serAddr;
	string ip;
	int port;
	bool c_bufferTimeout;
	int c_bufferlen;
public:
	SOCKET client;
	bool isConnect;
	int sendMsg(const char*buffer,const int ibufferlen = 0);
	int recvMsg(char *buffer,const int istimeout = 0);
	//int sendFile();
	int Init(string _ip, int _port);
	int Close();
};

struct FileInfo
{
	char *filename;
	int filesize;
	char mtime[21];
};