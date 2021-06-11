#ifndef __TCPCLASS_H
#define __TCPCLASS_H 1

#include <stdio.h>
#include <utime.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <limits.h>
#include <time.h>
#include <math.h>
#include <stdarg.h>
#include <errno.h>
#include <signal.h>
#include <netdb.h>
#include <locale.h>
#include <dirent.h>
#include <termios.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <iostream>
#include <string>
#include <cstdlib>
#include <cstring>
#include <list>
#include <vector>
#include <deque>
#include <algorithm>

// 采用stl标准库的命名空间std
using namespace std;

class TcpServer{
private:
	int s_socketlen;                //sockaddr_in结构体的长度
	struct sockaddr_in s_clientaddr; //客户端的地址信息
	struct sockaddr_in s_serveraddr;//服务端的地址信息

public:
	int s_listenfd;       //服务端用于监听的socket
	int s_connectfd;      //客户端连接上来的socket
	bool s_bufferTimeout;   //调用read和write方法时，是否超时
	int s_bufferLenth;    //调用read方法后，接收到的报文的大小，单位：字节
	int s_MAXCON;
	
	TcpServer();//构造函数
	
	/*
	*服务端初始化
	*port:指定服务端用于监听的端口
	*返回值：true,成功，false失败
	*/
	bool InitServer(const unsigned int port);
	
	/*
	*阻塞等待客户端的连接请求
	*返回值：true 有客户端连接请求， false Accept被中断
	*/
	bool Accept();
	
	/*
	*获取客户端的ip地址
	*返回值：客户端的ip地址，如："192.1680.0.1"
	*/
	char *GetIP();
	
	/*
	*接受客户端发来的数据
	*buffet:接受数据缓冲区的地址，数据长度存放在s_bufferlen成员变量中
	*istimeout:等待数据的超时时间，单位：秒，缺省是是0，无限等待
	*/
	bool Read(char* buffer, const int istimeout = 0);
	
	/*
	*向客户端发送数据
	*buffer：待发送数据缓冲区的地址
	*sendBuflen:待发送数据的大小，单位：字节,缺省是是0，如果发送的ascii字符串，取0，如果是二进制流数据，则为数据块的大小
	*返回值：true成功，false 失败，如果失败，说明socket连接已经不可使用
	*/
	bool Write(const char *buffer,const int sendBuflen = 0);
	
	void CloseListen();
	
	// 关闭客户端的socket
	void CloseClient();
	
	~TcpServer();	
};

class TcpClient
{
public:
	int c_socketfd; //客户端socket
	char c_ip[21];
	int c_port;
	bool c_bufferTimeout;
	int c_bufferlen;
	
	TcpClient();
	
	/*
	*向服务端发起连接请求
	*ip:服务端的ipd地址
	*port:服务端监听的端口
	*返回值：true false
	*/
	bool ConnectToServer(const char* ip, const int port);
	
	bool Read(char* buffer, const int istimeout=0);
	
	bool Write(const char *buffer, const int buflen=0);
	
	void Close();
	~TcpClient();
};


// 接收socket的对端发送过来的数据。
// sockfd：可用的socket连接。
// buffer：接收数据缓冲区的地址。
// ibuflen：本次成功接收数据的字节数。
// itimeout：接收等待超时的时间，单位：秒，缺省值是0-无限等待。
// 返回值：true-成功；false-失败，失败有两种情况：1）等待超时；2）socket连接已不可用。
bool TcpRead(const int sockfd,char *buffer,int *ibuflen,const int itimeout=0);

// 向socket的对端发送数据。
// sockfd：可用的socket连接。
// buffer：待发送数据缓冲区的地址。
// ibuflen：待发送数据的字节数，如果发送的是ascii字符串，ibuflen取0，如果是二进制流数据，ibuflen为二进制数据块的大小。
// 返回值：true-成功；false-失败，如果失败，表示socket连接已不可用。
bool TcpWrite(const int sockfd,const char *buffer,const int ibuflen=0);

// 从已经准备好的socket中读取数据。
// sockfd：已经准备好的socket连接。
// buffer：接收数据缓冲区的地址。
// n：本次接收数据的字节数。
// 返回值：成功接收到n字节的数据后返回true，socket连接不可用返回false。
bool Readn(const int sockfd,char *buffer,const size_t n);

// 向已经准备好的socket中写入数据。
// sockfd：已经准备好的socket连接。
// buffer：待发送数据缓冲区的地址。
// n：待发送数据的字节数。
// 返回值：成功发送完n字节的数据后返回true，socket连接不可用返回false。
bool Writen(const int sockfd,const char *buffer,const size_t n);


// 日志文件操作类
class CLogFile
{
public:
  FILE   *m_tracefp;           // 日志文件指针。
  char    m_filename[301];     // 日志文件名，建议采用绝对路径。
  char    m_openmode[11];      // 日志文件的打开方式，一般采用"a+"。
  bool    m_bEnBuffer;         // 写入日志时，是否启用操作系统的缓冲机制，缺省不启用。
  long    m_MaxLogSize;        // 最大日志文件的大小，单位M，缺省100M。
  bool    m_bBackup;           // 是否自动切换，日志文件大小超过m_MaxLogSize将自动切换，缺省启用。

  // 构造函数。
  // MaxLogSize：最大日志文件的大小，单位M，缺省100M，最小为10M。
  CLogFile(const long MaxLogSize=100);  

  // 打开日志文件。
  // filename：日志文件名，建议采用绝对路径，如果文件名中的目录不存在，就先创建目录。
  // openmode：日志文件的打开方式，与fopen库函数打开文件的方式相同，缺省值是"a+"。
  // bBackup：是否自动切换，true-切换，false-不切换，在多进程的服务程序中，如果多个进程共用一个日志文件，bBackup必须为false。
  // bEnBuffer：是否启用文件缓冲机制，true-启用，false-不启用，如果启用缓冲区，那么写进日志文件中的内容不会立即写入文件，缺省是不启用。
  bool Open(const char *filename,const char *openmode=0,bool bBackup=true,bool bEnBuffer=false);

  // 如果日志文件大于m_MaxLogSize的值，就把当前的日志文件名改为历史日志文件名，再创建新的当前日志文件。
  // 备份后的文件会在日志文件名后加上日期时间，如/tmp/log/filetodb.log.20200101123025。
  // 注意，在多进程的程序中，日志文件不可切换，多线的程序中，日志文件可以切换。
  bool BackupLogFile();

  // 把内容写入日志文件，fmt是可变参数，使用方法与printf库函数相同。
  // Write方法会写入当前的时间，WriteEx方法不写时间。
  bool Write(const char *fmt,...);
  bool WriteEx(const char *fmt,...);

  // 关闭日志文件
  void Close();

  ~CLogFile();  // 析构函数会调用Close方法。
};


//文件结构体信息
struct FileInfo
{
	char filename[301];
	int filesize;
	char mtime[21];
};

bool SendFile(int sockfd,struct FileInfo *fileinfo,CLogFile *logfile=0);

bool RecvFile(int sockfd,struct FileInfo *fileinfo,CLogFile *logfile=0);


bool UTime(const char *filename,const char *mtime);


FILE *FOPEN(const char *filename,const char *mode);

int FileSize(const char *filename);

bool FileMTime(const char *filename,char *mtime,const char *fmt=0);

bool RENAME(const char *srcfilename,const char *dstfilename,const int times=1);

time_t strtotime(const char *stime);

void LocalTime(char *stime,const char *fmt=0,const int timetvl=0);

bool MKDIR(const char *pathorfilename,bool bisfilename=true);

void PickNumber(const char *src,char *dest,const bool bsigned,const bool bdot);

void timetostr(const time_t ltime,char *stime,const char *fmt=0);

void DeleteLRChar(char *str,const char chr);

// 删除字符串左边指定的字符。
// str：待处理的字符串。
// chr：需要删除的字符。
void DeleteLChar(char *str,const char chr);

// 删除字符串右边指定的字符。
// str：待处理的字符串。
// chr：需要删除的字符。
void DeleteRChar(char *str,const char chr);

#endif