#include "WClient.h"


int WinClient::Init(string _ip, int _port){
	ip = _ip;
	port = _port;
	client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(client == INVALID_SOCKET)
		{
			printf("invalid socket!");
			return 0;
		}
	serAddr.sin_family = AF_INET;
	serAddr.sin_port = htons(port);
	const char *IP;
	IP = ip.c_str();
	serAddr.sin_addr.S_un.S_addr = inet_addr(IP);
	if(connect(client, (sockaddr *)&serAddr, sizeof(serAddr)) == SOCKET_ERROR)
	{  //连接失败 
		printf("connect error !");
		closesocket(client);
		return 0;
	}
	return 1;
}

int WinClient::recvMsg(char *buffer,const int istimeout){
	if (client== -1) return false;
	if (istimeout > 0){
		fd_set tempfd;
		FD_ZERO(&tempfd);
		FD_SET(client,&tempfd);
		struct timeval timeout;
		timeout.tv_sec = istimeout;
		timeout.tv_usec = 0;
		
		c_bufferTimeout = false;
		
		int i;
		if ((i = select(client+1,&tempfd,0,0,&timeout)) <= 0){
			if (i == 0) c_bufferTimeout = true;
			return false;
		}
	}
	c_bufferlen = 0;
	return (TcpRead(client,buffer,&c_bufferlen));
}

int WinClient::sendMsg(const char*buffer,const int ibufferlen){
	if (client == -1) return false;
	fd_set tempfd;
	FD_ZERO(&tempfd);
	FD_SET(client,&tempfd);
	
	struct timeval timeout;
	timeout.tv_sec = 5; timeout.tv_usec = 0;
	
	c_bufferTimeout = false;
	
	int i;
	if ( (i=select(client+1,0,&tempfd,0,&timeout)) <= 0 )
	{
		if (i==0) c_bufferTimeout = true;
		return false;
	}
	int ilen = ibufferlen;
	
	if (ibufferlen == 0) ilen = strlen(buffer);
	return (TcpWrite(client,buffer,ilen));
}

int WinClient::Close(){
	closesocket(client);
	return 1;
}

bool Readn(const int sockfd,char *buffer,const size_t n)
{
  int nLeft,nread,idx;

  nLeft = n;
  idx = 0;

  while(nLeft > 0)
  {
    if ( (nread = recv(sockfd,buffer + idx,nLeft,0)) <= 0) return false;

    idx += nread;
    nLeft -= nread;
  }

  return true;
}

bool Writen(const int sockfd,const char *buffer,const size_t n)
{
  int nLeft,idx,nwritten;
  nLeft = n;  
  idx = 0;
  while(nLeft > 0 )
  {    
    if ( (nwritten = send(sockfd, buffer + idx,nLeft,0)) <= 0) return false;      
    
    nLeft -= nwritten;
    idx += nwritten;
  }

  return true;
}

bool TcpRead(const int sockfd,char *buffer,int *ibuflen,const int itimeout)
{
  if (sockfd == -1) return false;

  if (itimeout > 0)
  {
    fd_set tmpfd;

    FD_ZERO(&tmpfd);
    FD_SET(sockfd,&tmpfd);

    struct timeval timeout;
    timeout.tv_sec = itimeout; timeout.tv_usec = 0;

    int i;
    if ( (i = select(sockfd+1,&tmpfd,0,0,&timeout)) <= 0 ) return false;
  }

  (*ibuflen) = 0;

  if (Readn(sockfd,(char*)ibuflen,4) == false) return false;

  (*ibuflen)=ntohl(*ibuflen);  // 把网络字节序转换为主机字节序。

  if (Readn(sockfd,buffer,(*ibuflen)) == false) return false;

  return true;
}

bool TcpWrite(const int sockfd,const char *buffer,const int ibuflen)
{
  if (sockfd == -1) return false;

  fd_set tmpfd;

  FD_ZERO(&tmpfd);
  FD_SET(sockfd,&tmpfd);

  struct timeval timeout;
  timeout.tv_sec = 5; timeout.tv_usec = 0;

  if ( select(sockfd+1,0,&tmpfd,0,&timeout) <= 0 ) return false;
  
  int ilen=0;

  // 如果长度为0，就采用字符串的长度
  if (ibuflen==0) ilen=strlen(buffer);
  else ilen=ibuflen;

  int ilenn=htonl(ilen);  // 转换为网络字节序。

  char strTBuffer[1024+4];
  memset(strTBuffer,0,sizeof(strTBuffer));
  memcpy(strTBuffer,&ilenn,4);
  memcpy(strTBuffer+4,buffer,ilen);
  
  if (Writen(sockfd,strTBuffer,ilen+4) == false) return false;

  return true;
}

string getName(const char* full_name)
{
	string file_name = full_name;
	const char*  mn_first = full_name;
	const char*  mn_last = full_name + strlen(full_name);
	if (strrchr(full_name, '\\') != NULL)
		mn_first = strrchr(full_name, '\\') + 1;
	else if (strrchr(full_name, '/') != NULL)
		mn_first = strrchr(full_name, '/') + 1;
	if (mn_last < mn_first)
		mn_last = full_name + strlen(full_name);

	file_name.assign(mn_first, mn_last);

	return file_name;
}

bool SendFile(int sockfd,struct FileInfo *fileinfo)
{
  char strSendBuffer[301],strRecvBuffer[301];
  memset(strSendBuffer,0,sizeof(strSendBuffer));

  snprintf(strSendBuffer,300,"%s %d %s",getName(fileinfo->filename).data(),fileinfo->filesize,fileinfo->mtime);

  if (TcpWrite(sockfd,strSendBuffer) == false)
  {
    printf("SendFile TcpWrite() failed.\n"); 
    return false;
  }

  int  bytes=0;
  int  total_bytes=0;
  int  onread=0;
  char buffer[1000];

  FILE *fp=0;

  if ( (fp=FOPEN(fileinfo->filename,"rb")) == 0 )
  {
    printf("SendFile FOPEN(%s) failed.\n",fileinfo->filename); 
    return false;
  }

  while (true)
  {
    memset(buffer,0,sizeof(buffer));

    if ((fileinfo->filesize-total_bytes) > 1000) onread=1000;
    else onread=fileinfo->filesize-total_bytes;

    bytes=fread(buffer,1,onread,fp);

    if (bytes > 0)
    {
      if (Writen(sockfd,buffer,bytes) == false)
      {
        printf("SendFile Writen() failed.\n"); 
        fclose(fp); fp=0; return false;
      }
    }

    total_bytes = total_bytes + bytes;

    if ((int)total_bytes == fileinfo->filesize) break;
  }

  fclose(fp);

  // 接收对端返回的确认报文
  int buflen=0;
  memset(strRecvBuffer,0,sizeof(strRecvBuffer));
  if (TcpRead(sockfd,strRecvBuffer,&buflen)==false)
  {
    printf("SendFile TcpRead() failed.\n"); 
    return false;
  }

  if (strcmp(strRecvBuffer,"ok")!=0) return false;

  return true;
}

FILE *FOPEN(const char *filename,const char *mode)
{
  if (MKDIR(filename) == false) return 0;

  return fopen(filename,mode);
}

int FileSize(const char *filename)
{
  struct _stat st_filestat;

  if (_stat(filename,&st_filestat) < 0) return -1;

  return st_filestat.st_size;
}

bool MKDIR(const char *filename,bool bisfilename)
{
  // 检查目录是否存在，如果不存在，逐级创建子目录
  char strPathName[301];

  int ilen=strlen(filename);

  for (int ii=1; ii<ilen;ii++)
  {
    if (filename[ii] != '/') continue;

    memset(strPathName,0,sizeof(strPathName));
    strncpy(strPathName,filename,ii);

    if (_access(strPathName,0) == 0) continue;

    if (_mkdir(strPathName) != 0) return false;
  }

  if (bisfilename==false)
  {
    if (_access(filename,0) != 0)
    {
      if (_mkdir(filename) != 0) return false;
    }
  }

  return true;
}

void LocalTime(char *stime,const char *fmt,const int timetvl)
{
  if (stime==0) return;

  time_t  timer;

  time( &timer ); timer=timer+timetvl;

  timetostr(timer,stime,fmt);
}

void timetostr(const time_t ltime,char *stime,const char *fmt)
{
  if (stime==0) return;

  strcpy(stime,"");

  struct tm sttm = *localtime ( &ltime ); 

  sttm.tm_year=sttm.tm_year+1900;
  sttm.tm_mon++;

  if (fmt==0)
  {
    snprintf(stime,20,"%04u-%02u-%02u %02u:%02u:%02u",sttm.tm_year,
                    sttm.tm_mon,sttm.tm_mday,sttm.tm_hour,
                    sttm.tm_min,sttm.tm_sec);
    return;
  }

  if (strcmp(fmt,"yyyy-mm-dd hh24:mi:ss") == 0)
  {
    snprintf(stime,20,"%04u-%02u-%02u %02u:%02u:%02u",sttm.tm_year,
                    sttm.tm_mon,sttm.tm_mday,sttm.tm_hour,
                    sttm.tm_min,sttm.tm_sec);
    return;
  }

  if (strcmp(fmt,"yyyy-mm-dd hh24:mi") == 0)
  {
    snprintf(stime,17,"%04u-%02u-%02u %02u:%02u",sttm.tm_year,
                    sttm.tm_mon,sttm.tm_mday,sttm.tm_hour,
                    sttm.tm_min);
    return;
  }

  if (strcmp(fmt,"yyyy-mm-dd hh24") == 0)
  {
    snprintf(stime,14,"%04u-%02u-%02u %02u",sttm.tm_year,
                    sttm.tm_mon,sttm.tm_mday,sttm.tm_hour);
    return;
  }

  if (strcmp(fmt,"yyyy-mm-dd") == 0)
  {
    snprintf(stime,11,"%04u-%02u-%02u",sttm.tm_year,sttm.tm_mon,sttm.tm_mday); 
    return;
  }

  if (strcmp(fmt,"yyyy-mm") == 0)
  {
    snprintf(stime,8,"%04u-%02u",sttm.tm_year,sttm.tm_mon); 
    return;
  }

  if (strcmp(fmt,"yyyymmddhh24miss") == 0)
  {
    snprintf(stime,15,"%04u%02u%02u%02u%02u%02u",sttm.tm_year,
                    sttm.tm_mon,sttm.tm_mday,sttm.tm_hour,
                    sttm.tm_min,sttm.tm_sec);
    return;
  }

  if (strcmp(fmt,"yyyymmddhh24mi") == 0)
  {
    snprintf(stime,13,"%04u%02u%02u%02u%02u",sttm.tm_year,
                    sttm.tm_mon,sttm.tm_mday,sttm.tm_hour,
                    sttm.tm_min);
    return;
  }

  if (strcmp(fmt,"yyyymmddhh24") == 0)
  {
    snprintf(stime,11,"%04u%02u%02u%02u",sttm.tm_year,
                    sttm.tm_mon,sttm.tm_mday,sttm.tm_hour);
    return;
  }

  if (strcmp(fmt,"yyyymmdd") == 0)
  {
    snprintf(stime,9,"%04u%02u%02u",sttm.tm_year,sttm.tm_mon,sttm.tm_mday); 
    return;
  }

  if (strcmp(fmt,"hh24miss") == 0)
  {
    snprintf(stime,7,"%02u%02u%02u",sttm.tm_hour,sttm.tm_min,sttm.tm_sec); 
    return;
  }

  if (strcmp(fmt,"hh24mi") == 0)
  {
    snprintf(stime,5,"%02u%02u",sttm.tm_hour,sttm.tm_min); 
    return;
  }

  if (strcmp(fmt,"hh24") == 0)
  {
    snprintf(stime,3,"%02u",sttm.tm_hour); 
    return;
  }

  if (strcmp(fmt,"mi") == 0)
  {
    snprintf(stime,3,"%02u",sttm.tm_min); 
    return;
  }
}

bool FileMTime(const char *filename,char *mtime,const char *fmt)
{
  // 判断文件是否存在。
  struct stat st_filestat;

  if (stat(filename,&st_filestat) < 0) return false;

  char strfmt[25];
  memset(strfmt,0,sizeof(strfmt));
  if (fmt==0) strcpy(strfmt,"yyyymmddhh24miss");
  else strcpy(strfmt,fmt);

  timetostr(st_filestat.st_mtime,mtime,strfmt);

  return true;
}

void showMenu(){
	printf("choose what do you want to do:\n");
	printf("1.send message\n");
	printf("2.send file\n");
}
