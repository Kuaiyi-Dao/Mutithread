#include "TcpClass.h"

TcpClient::TcpClient(){
	c_socketfd = -1;
	memset(c_ip,0,sizeof(c_ip));
	c_port = 0;
	c_bufferTimeout=false;
}
/*
	int c_socketfd; //客户端socket
	char c_ip[21];
	int c_port;
	bool c_bufferTimeout;
	int c_bufferlen;
*/

bool TcpClient::ConnectToServer(const char* ip, const int port){
	if (c_socketfd != -1) 
	{
		close(c_socketfd);
		c_socketfd = -1;
	}
	strcpy(c_ip,ip);
	c_port = port;
	struct hostent* h;
	struct sockaddr_in serveraddr;
	
	if ((c_socketfd = socket(AF_INET,SOCK_STREAM,0)) < 0) return false;
	
	if (! (h = gethostbyname(c_ip))){
		close(c_socketfd);
		c_socketfd = -1;
		return false;
	}
	memset(&serveraddr,0,sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(c_port);
	memcpy(&serveraddr.sin_addr,h->h_addr,h->h_length);
	if (connect(c_socketfd,(struct sockaddr *)&serveraddr,sizeof(serveraddr)))
	{
		close(c_socketfd);
		c_socketfd = -1;
		return false;
	}
	return true;
}

bool TcpClient::Read(char *buffer,const int istimeout){
	if (c_socketfd == -1) return false;
	
	if (istimeout > 0){
		fd_set tempfd;
		FD_ZERO(&tempfd);
		FD_SET(c_socketfd,&tempfd);
		struct timeval timeout;
		timeout.tv_sec = istimeout;
		timeout.tv_usec = 0;
		
		c_bufferTimeout = false;
		
		int i;
		if ((i = select(c_socketfd+1,&tempfd,0,0,&timeout)) <= 0){
			if (i == 0) c_bufferTimeout = true;
			return false;
		}
	}
	c_bufferlen = 0;
	return (TcpRead(c_socketfd,buffer,&c_bufferlen));
	
}

bool TcpClient::Write(const char*buffer,const int ibufferlen){
	if (c_socketfd == -1) return false;
	fd_set tempfd;
	FD_ZERO(&tempfd);
	FD_SET(c_socketfd,&tempfd);
	
	struct timeval timeout;
	timeout.tv_sec = 5; timeout.tv_usec = 0;
	
	c_bufferTimeout = false;
	
	int i;
	if ( (i=select(c_socketfd+1,0,&tempfd,0,&timeout)) <= 0 )
	{
		if (i==0) c_bufferTimeout = true;
		return false;
	}
	int ilen = ibufferlen;
	
	if (ibufferlen == 0) ilen = strlen(buffer);
	return (TcpWrite(c_socketfd,buffer,ilen));
}

void TcpClient::Close(){
	if (c_socketfd > 0) close(c_socketfd); 
	c_socketfd=-1;
	memset(c_ip,0,sizeof(c_ip));
	c_port = 0;
	c_bufferTimeout=false;
}

TcpClient::~TcpClient()
{
  Close();
}

TcpServer::TcpServer()
{
  s_listenfd=-1;
  s_connectfd=-1;
  s_bufferLenth=0;
  s_bufferTimeout=false;
  s_MAXCON = 5;
}
/*
	int s_listenfd;       //服务端用于监听的socket
	int s_connectfd;      //客户端连接上来的socket
	bool s_bufferTimeout;   //调用read和write方法时，是否超时
	int s_bufferLenth;
*/

bool TcpServer::InitServer(const unsigned int port){
	if (s_listenfd > 0){
		close(s_listenfd);
		s_listenfd = -1;
	}
	s_listenfd = socket(AF_INET,SOCK_STREAM,0);
	int opt = 1;
	unsigned int len = sizeof(opt);
	setsockopt(s_listenfd,SOL_SOCKET,SO_REUSEADDR,&opt,len);
	
	memset(&s_serveraddr,0,sizeof(s_serveraddr));
	s_serveraddr.sin_family = AF_INET;
	s_serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	s_serveraddr.sin_port = htons(port);
	if (bind(s_listenfd,(struct sockaddr*)&s_serveraddr,sizeof(s_serveraddr)) != 0){
		CloseListen();
		return false;
	}
	if (listen(s_listenfd,s_MAXCON) != 0)
	{
		CloseListen();
		return false;
	}
	s_bufferLenth = sizeof(struct sockaddr_in);
	
	return true;
}

bool TcpServer::Accept()
{
	if(s_listenfd == -1) return false;
	
	if ((s_connectfd = accept(s_listenfd,(struct sockaddr*)&s_clientaddr,(socklen_t*)&s_bufferLenth)) < 0)
	{
		return false;
	}
	return true;
}

char *TcpServer::GetIP(){
	return (inet_ntoa(s_clientaddr.sin_addr));
}

bool TcpServer::Read(char *buffer,const int itimeout)
{
  if (s_connectfd == -1) return false;

  if (itimeout>0)
  {
    fd_set tempfd;

    FD_ZERO(&tempfd);
    FD_SET(s_connectfd,&tempfd);

    struct timeval timeout;
    timeout.tv_sec = itimeout; timeout.tv_usec = 0;

    s_bufferTimeout = false;

    int i;
    if ( (i = select(s_connectfd+1,&tempfd,0,0,&timeout)) <= 0 )
    {
      if (i==0) s_bufferTimeout = true;
      return false;
    }
  }

  s_bufferLenth = 0;
  return(TcpRead(s_connectfd,buffer,&s_bufferLenth));
}

bool TcpServer::Write(const char *buffer,const int ibuflen)
{
  if (s_connectfd == -1) return false;

  fd_set tempfd;

  FD_ZERO(&tempfd);
  FD_SET(s_connectfd,&tempfd);

  struct timeval timeout;
  timeout.tv_sec = 5; timeout.tv_usec = 0;
  
  s_bufferTimeout = false;

  int i;
  if ( (i=select(s_connectfd+1,0,&tempfd,0,&timeout)) <= 0 )
  {
    if (i==0) s_bufferTimeout = true;
    return false;
  }

  int ilen = ibuflen;
  if (ilen==0) ilen=strlen(buffer);

  return(TcpWrite(s_connectfd,buffer,ilen));
}

void TcpServer::CloseListen()
{
  if (s_listenfd > 0)
  {
    close(s_listenfd); s_listenfd=-1;
  }
}

void TcpServer::CloseClient()
{
  if (s_connectfd > 0)
  {
    close(s_connectfd); s_connectfd=-1; 
  }
}

TcpServer::~TcpServer()
{
  CloseListen(); CloseClient();
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

  char strTBuffer[ilen+4];
  memset(strTBuffer,0,sizeof(strTBuffer));
  memcpy(strTBuffer,&ilenn,4);
  memcpy(strTBuffer+4,buffer,ilen);
  
  if (Writen(sockfd,strTBuffer,ilen+4) == false) return false;

  return true;
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

bool SendFile(int sockfd,struct FileInfo *fileinfo,CLogFile *logfile)
{
  char strSendBuffer[301],strRecvBuffer[301];
  memset(strSendBuffer,0,sizeof(strSendBuffer));

  snprintf(strSendBuffer,300,"%s %d %s",fileinfo->filename,fileinfo->filesize,fileinfo->mtime);

  if (TcpWrite(sockfd,strSendBuffer) == false)
  {
    if (logfile!=0) logfile->Write("SendFile TcpWrite() failed.\n"); 
    return false;
  }

  int  bytes=0;
  int  total_bytes=0;
  int  onread=0;
  char buffer[1000];

  FILE *fp=0;

  if ( (fp=FOPEN(fileinfo->filename,"rb")) == 0 )
  {
    if (logfile!=0) logfile->Write("SendFile FOPEN(%s) failed.\n",fileinfo->filename); 
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
        if (logfile!=0) logfile->Write("SendFile Writen() failed.\n"); 
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
    if (logfile!=0) logfile->Write("SendFile TcpRead() failed.\n"); 
    return false;
  }

  if (strcmp(strRecvBuffer,"ok")!=0) return false;

  return true;
}

// 接收通过socdfd发送过来的文件
bool RecvFile(int sockfd,struct FileInfo *fileinfo,CLogFile *logfile)
{
  char strSendBuffer[301];

  char strfilenametmp[301]; memset(strfilenametmp,0,sizeof(strfilenametmp));
  sprintf(strfilenametmp,"%s.tmp",fileinfo->filename);

  FILE *fp=0;

  if ( (fp=FOPEN(strfilenametmp,"wb")) ==0)     // FOPEN可创建目录
  {
    if (logfile!=0) logfile->Write("RecvFile FOPEN %s failed.\n",strfilenametmp); 
    return false;
  }

  int  total_bytes=0;
  int  onread=0;
  char buffer[1000];

  while (true)
  {
    memset(buffer,0,sizeof(buffer));

    if ((fileinfo->filesize-total_bytes) > 1000) onread=1000;
    else onread=fileinfo->filesize-total_bytes;

    if (Readn(sockfd,buffer,onread) == false)
    {
      if (logfile!=0) logfile->Write("RecvFile Readn() failed.\n"); 
      fclose(fp); fp=0; return false;
    }

    fwrite(buffer,1,onread,fp);

    total_bytes = total_bytes + onread;

    if ((int)total_bytes == fileinfo->filesize) break;
  }

  fclose(fp);

  // 重置文件的时间
  UTime(strfilenametmp,fileinfo->mtime);

  memset(strSendBuffer,0,sizeof(strSendBuffer));
  if (RENAME(strfilenametmp,fileinfo->filename)==true) strcpy(strSendBuffer,"ok");
  else strcpy(strSendBuffer,"failed");

  // 向对端返回响应内容
  if (TcpWrite(sockfd,strSendBuffer)==false)
  {
    if (logfile!=0) logfile->Write("RecvFile TcpWrite() failed.\n"); 
    return false;
  }

  if (strcmp(strSendBuffer,"ok") != 0) return false;

  return true;
}


CLogFile::CLogFile(const long MaxLogSize)
{
  m_tracefp = 0;
  memset(m_filename,0,sizeof(m_filename));
  memset(m_openmode,0,sizeof(m_openmode));
  m_bBackup=true;
  m_bEnBuffer=false;
  m_MaxLogSize=MaxLogSize;
  if (m_MaxLogSize<10) m_MaxLogSize=10;
}

CLogFile::~CLogFile()
{
  Close();
}

void CLogFile::Close()
{
  if (m_tracefp != 0) { fclose(m_tracefp); m_tracefp=0; }

  memset(m_filename,0,sizeof(m_filename));
  memset(m_openmode,0,sizeof(m_openmode));
  m_bBackup=true;
  m_bEnBuffer=false;
}

// 打开日志文件。
// filename：日志文件名，建议采用绝对路径，如果文件名中的目录不存在，就先创建目录。
// openmode：日志文件的打开方式，与fopen库函数打开文件的方式相同，缺省值是"a+"。
// bBackup：是否自动切换，true-切换，false-不切换，在多进程的服务程序中，如果多个进行共用一个日志文件，bBackup必须为false。
// bEnBuffer：是否启用文件缓冲机制，true-启用，false-不启用，如果启用缓冲区，那么写进日志文件中的内容不会立即写入文件，缺省是不启用。
bool CLogFile::Open(const char *filename,const char *openmode,bool bBackup,bool bEnBuffer)
{
  // 如果文件指针是打开的状态，先关闭它。
  Close();

  strcpy(m_filename,filename);
  m_bEnBuffer=bEnBuffer;
  m_bBackup=bBackup;
  if (openmode==0) strcpy(m_openmode,"a+");
  else strcpy(m_openmode,openmode);

  if ((m_tracefp=FOPEN(m_filename,m_openmode)) == 0) return false;

  return true;
}

// 如果日志文件大于100M，就把当前的日志文件备份成历史日志文件，切换成功后清空当前日志文件的内容。
// 备份后的文件会在日志文件名后加上日期时间。
// 注意，在多进程的程序中，日志文件不可切换，多线的程序中，日志文件可以切换。
bool CLogFile::BackupLogFile()
{
  if (m_tracefp == 0) return false;

  // 不备份
  if (m_bBackup == false) return true;

  fseek(m_tracefp,0L,2);

  if (ftell(m_tracefp) > m_MaxLogSize*1024*1024)
  {
    fclose(m_tracefp); m_tracefp=0;

    char strLocalTime[21];
    memset(strLocalTime,0,sizeof(strLocalTime));
    LocalTime(strLocalTime,"yyyymmddhh24miss");

    char bak_filename[301];
    memset(bak_filename,0,sizeof(bak_filename));
    snprintf(bak_filename,300,"%s.%s",m_filename,strLocalTime);
    rename(m_filename,bak_filename);

    if ((m_tracefp=FOPEN(m_filename,m_openmode)) == 0) return false;
  }

  return true;
}

// 把内容写入日志文件，fmt是可变参数，使用方法与printf库函数相同。
// Write方法会写入当前的时间，WriteEx方法不写时间。
bool CLogFile::Write(const char *fmt,...)
{
  if (m_tracefp == 0) return false;

  if (BackupLogFile() == false) return false;

  char strtime[20]; LocalTime(strtime);

  va_list ap;
  va_start(ap,fmt);
  fprintf(m_tracefp,"%s ",strtime);
  vfprintf(m_tracefp,fmt,ap);
  va_end(ap);

  if (m_bEnBuffer==false) fflush(m_tracefp);

  return true;
}

// 把内容写入日志文件，fmt是可变参数，使用方法与printf库函数相同。
// Write方法会写入当前的时间，WriteEx方法不写时间。
bool CLogFile::WriteEx(const char *fmt,...)
{
  if (m_tracefp == 0) return false;

  va_list ap;
  va_start(ap,fmt);
  vfprintf(m_tracefp,fmt,ap);
  va_end(ap);

  if (m_bEnBuffer==false) fflush(m_tracefp);

  return true;
}

bool UTime(const char *filename,const char *mtime)
{
  struct utimbuf stutimbuf;

  stutimbuf.actime=stutimbuf.modtime=strtotime(mtime);

  if (utime(filename,&stutimbuf)!=0) return false;

  return true;
}

FILE *FOPEN(const char *filename,const char *mode)
{
  if (MKDIR(filename) == false) return 0;

  return fopen(filename,mode);
}


int FileSize(const char *filename)
{
  struct stat st_filestat;

  if (stat(filename,&st_filestat) < 0) return -1;

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

    if (access(strPathName,F_OK) == 0) continue;

    if (mkdir(strPathName,0755) != 0) return false;
  }

  if (bisfilename==false)
  {
    if (access(filename,F_OK) != 0)
    {
      if (mkdir(filename,0755) != 0) return false;
    }
  }

  return true;
}

// 把字符串表示的时间转换为整数表示的时间。
// stime：字符串表示的时间，格式不限，但一定要包括yyyymmddhh24miss，一个都不能少。
// 返回值：整数表示的时间，如果stime的格式不正确，返回-1。
time_t strtotime(const char *stime)
{
  char strtime[21],yyyy[5],mm[3],dd[3],hh[3],mi[3],ss[3];
  memset(strtime,0,sizeof(strtime));
  memset(yyyy,0,sizeof(yyyy));
  memset(mm,0,sizeof(mm));
  memset(dd,0,sizeof(dd));
  memset(hh,0,sizeof(hh));
  memset(mi,0,sizeof(mi));
  memset(ss,0,sizeof(ss));

  PickNumber(stime,strtime,false,false);

  if (strlen(strtime) != 14) return -1;

  strncpy(yyyy,strtime,4);
  strncpy(mm,strtime+4,2);
  strncpy(dd,strtime+6,2);
  strncpy(hh,strtime+8,2);
  strncpy(mi,strtime+10,2);
  strncpy(ss,strtime+12,2);

  struct tm time_str;

  time_str.tm_year = atoi(yyyy) - 1900;
  time_str.tm_mon = atoi(mm) - 1;
  time_str.tm_mday = atoi(dd);
  time_str.tm_hour = atoi(hh);
  time_str.tm_min = atoi(mi);
  time_str.tm_sec = atoi(ss);
  time_str.tm_isdst = 0;

  return mktime(&time_str);
}

void LocalTime(char *stime,const char *fmt,const int timetvl)
{
  if (stime==0) return;

  time_t  timer;

  time( &timer ); timer=timer+timetvl;

  timetostr(timer,stime,fmt);
}

// 把文件重命名，类似Linux系统的mv命令。
// srcfilename：原文件名，建议采用绝对路径的文件名。
// destfilename：目标文件名，建议采用绝对路径的文件名。
// times：执行重命名文件的次数，缺省是1，建议不要超过3，从实际应用的经验看来，如果重命名文件第1次不成功，再尝
// 试2次是可以的，更多次就意义不大了。还有，如果执行重命名失败，usleep(100000)后再重试。
// 返回值：true-重命名成功；false-重命名失败，失败的主要原因是权限不足或磁盘空间不够，如果原文件和目标文件不
// 在同一个磁盘分区，重命名也可能失败。
// 注意，在重命名文件之前，会自动创建destfilename参数中的目录名。
// 在应用开发中，可以用RENAME函数代替rename库函数。
bool RENAME(const char *srcfilename,const char *dstfilename,const int times)
{
  // 如果文件不存在，直接返回失败
  if (access(srcfilename,R_OK) != 0) return false;

  if (MKDIR(dstfilename) == false) return false;

  for (int ii=0;ii<times;ii++)
  {
    if (rename(srcfilename,dstfilename) == 0) return true;

    usleep(100000);
  }

  return false;
}

void PickNumber(const char *src,char *dest,const bool bsigned,const bool bdot)
{
  if (dest==0) return;
  if (src==0) { strcpy(dest,""); return; }

  char strtemp[strlen(src)+1];
  memset(strtemp,0,sizeof(strtemp));
  strcpy(strtemp,src);
  DeleteLRChar(strtemp,' ');

  int ipossrc,iposdst,ilen;
  ipossrc=iposdst=ilen=0;

  ilen=strlen(strtemp);

  for (ipossrc=0;ipossrc<ilen;ipossrc++)
  {
    if ( (bsigned==true) && (strtemp[ipossrc] == '+') )
    {
      dest[iposdst++]=strtemp[ipossrc]; continue;
    }

    if ( (bsigned==true) && (strtemp[ipossrc] == '-') )
    {
      dest[iposdst++]=strtemp[ipossrc]; continue;
    }

    if ( (bdot==true) && (strtemp[ipossrc] == '.') )
    {
      dest[iposdst++]=strtemp[ipossrc]; continue;
    }

    if (isdigit(strtemp[ipossrc])) dest[iposdst++]=strtemp[ipossrc];
  }

  dest[iposdst]=0;
}


// 把整数表示的时间转换为字符串表示的时间。
// ltime：整数表示的时间。
// stime：字符串表示的时间。
// fmt：输出字符串时间stime的格式，与LocalTime函数的fmt参数相同，如果fmt的格式不正确，stime将为空。
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

void DeleteLRChar(char *str,const char chr)
{
  DeleteLChar(str,chr);
  DeleteRChar(str,chr);
}

// 删除字符串左边指定的字符。
// str：待处理的字符串。
// chr：需要删除的字符。
void DeleteLChar(char *str,const char chr)
{
  if (str == 0) return;
  if (strlen(str) == 0) return;

  char strTemp[strlen(str)+1];

  int iTemp=0;

  memset(strTemp,0,sizeof(strTemp));
  strcpy(strTemp,str);

  while ( strTemp[iTemp] == chr )  iTemp++;

  memset(str,0,strlen(str)+1);

  strcpy(str,strTemp+iTemp);

  return;
}

// 删除字符串右边指定的字符。
// str：待处理的字符串。
// chr：需要删除的字符。
void DeleteRChar(char *str,const char chr)
{
  if (str == 0) return;
  if (strlen(str) == 0) return;

  int istrlen = strlen(str);

  while (istrlen>0)
  {
    if (str[istrlen-1] != chr) break;

    str[istrlen-1]=0;

    istrlen--;
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
