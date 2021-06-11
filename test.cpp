#include "TcpClass.h"
#include "Pthread_Pool.h"

void pthMain(void * arg);

int main(){
	TcpServer server;
	if (!server.InitServer(8086)) printf("init failed");
	ThreadPool *pool = new ThreadPool(3,3);
	while(true){
		if(server.Accept()==false)
		{
			printf("socket create failed!\n"); return -1;
		}
		printf("%s已连接\n", server.GetIP());
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
		if (TcpRead(socketfd,buffer,&lenth,300) == false) break;
		printf("接收到：%s\n", buffer);
		if (memcmp(buffer,"2",1) == 0){
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
			if (RecvFile(socketfd,&recvFile)) printf("已接收文件：%s\n",recvFile.filename);
			//printf("接收到：%s\n", buffer);
			//memset(&buffer,0,sizeof(buffer));
		}else{
		const char*response="OK";
		if (TcpWrite(socketfd,response,300) == false) break;
		}
		
	}
	printf("客户端已断开\n");
}
//todo 
//	   客户机身份认证 {通过ip地址来实现}
// 	   客户机管理链表 记录客户机运行pthread (可用一个线程来管理)
//	   	   