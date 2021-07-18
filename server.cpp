#include "TcpClass.h"
#include "Pthread_Pool.h"
#include "IDList.h"
#define TIMEOUT 30

void pthMain(void * arg);
static CetID *clientList = new CetID();

int main(int argc,char* argv[]){
	int maxThread;
	if (argc < 2)
		maxThread = 3;
	else
	{
		maxThread = atoi((const char*)argv[1]);
		if (maxThread < 1)
		{
			printf("最大线程数无效\n");
			return -1;
		}
	}
	TcpServer server;
	if (!server.InitServer(8086)) printf("init failed");
	ThreadPool *pool = new ThreadPool(1,maxThread);

	while(true){
		if(server.Accept()==false)
		{
			printf("socket create failed!\n"); return -1;
		}
		//身份认证
		if (clientList->IsInWL(server.GetIP()))
		{
			printf("%s已连接\n", server.GetIP());
			clientList->AddIp(server.GetIP(), server.s_connectfd);
			server.Write("accept");
		}
		else{
			printf("拒绝%s连接\n",server.GetIP());
			server.Write("refuse");
			continue;
		}
		Task task = {pthMain,(void*)server.s_connectfd,NULL,NULL};
		pool->TaskAdd(&task);
	}
	server.CloseListen();
	return 0;
}

void pthMain(void * arg){
	int socketfd = (int)(long)arg;
	char buffer[1024];
	memset(&buffer,0,sizeof(buffer));
	while(true)
	{	
		int lenth;
		memset(&buffer,0,sizeof(buffer));
		if (TcpRead(socketfd,buffer,&lenth,TIMEOUT) == false)
		{
			clientList->RmIp(socketfd);
			printf("删除%d\n",socketfd);
			break;
		}
		//文件接收
		if (memcmp(buffer,"2",1) == 0){
			time_t t = getTime();
			printf("%d开始发送文件 %s\n",socketfd,asctime(gmtime(&t)));
			memset(&buffer,0,sizeof(buffer));
			const char*response="OK";
			if (TcpWrite(socketfd,response,300) == false) break;
			TcpRead(socketfd,buffer,&lenth,300);
			struct FileInfo recvFile;
			memset(&recvFile,0,sizeof(FileInfo));
			//recvFile.filesize = FileSize((const char*)&(recvFile.filename));
			char * name;
			char * size;
			char * time;
			char *p = buffer;
			name = strsep(&p," ");
			size = strsep(&p," ");
			time = strsep(&p," ");
			recvFile.filesize = atoi((const char*)size);
			memcpy(&(recvFile.filename),strcat(name,"bak"),strlen(strcat(name,"bak")));
			memcpy(&(recvFile.mtime),time,strlen(time)+1);
			if (RecvFile(socketfd,&recvFile))
			{
				time_t t1 = getTime();
				system("ps H -eo user,pid,ppid,tid,time,%cpu,cmd --sort=%cpu");
				printf("已接收%d的文件：%s %s\n",socketfd,recvFile.filename,asctime(gmtime(&t1)));
			}
			//printf("接收到：%s\n", buffer);
			//memset(&buffer,0,sizeof(buffer));
		}else if (memcmp(buffer,"1",1) == 0){
			time_t t2 = getTime();
			const char*response="OK";
			if (TcpWrite(socketfd,response,300) == false) break;
			memset(&buffer,0,sizeof(buffer));
			if (TcpRead(socketfd,buffer,&lenth,300) == false) break;
			printf("%d发送：%s %s\n",socketfd,buffer,asctime(gmtime(&t2)));
			strcat(buffer,"--OK");
			if (TcpWrite(socketfd,buffer,300) == false) break;
		}
	}
	printf("%d客户端已断开\n",socketfd);
}