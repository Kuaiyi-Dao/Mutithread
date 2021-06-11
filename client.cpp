#include "TcpClass.h"

void showMenu();

int main(){
	TcpClient client;
	client.ConnectToServer("127.0.0.1",8086);
	char buffer[1024];
	char choice[2];
	memset(&buffer,0,sizeof(buffer));
	client.Read(buffer);
	if (strcmp(buffer,"accept") != 0){
		printf("已被服务器拒绝连接\n");
		sleep(3);
		return 0;
	}
	while(true)
	{
		memset(&buffer,0,sizeof(buffer));
		//sprintf(buffer,"这是%d第%d次消息",getpid(),i+1);
		showMenu();
		scanf("%s",&choice);
		if (choice[0] == '1'){
			client.Write((const char*)choice);
			scanf("%s",&buffer);
			client.Write(buffer);
			memset(&buffer,0,sizeof(buffer));
			client.Read(buffer);
			printf("%s\n", buffer);
		}else if (choice[0] == '2'){
			memset(&buffer,0,sizeof(buffer));
			struct FileInfo sendFile;
			memset(&sendFile,0,sizeof(FileInfo));
			scanf("%s",&(sendFile.filename));
			sendFile.filesize = FileSize((const char*)&(sendFile.filename));
			if (sendFile.filesize == -1){
				printf("没有此文件\n");
				continue;
			}
			client.Write((const char*)choice);
			if(client.Read(buffer))
			{
				FileMTime((const char*)&(sendFile.filename),(char*)&(sendFile.mtime));
				if (SendFile(client.c_socketfd,&sendFile)) printf("已发送文件：%s\n",sendFile.filename);
			}
		}else continue;
		//client.Write(buffer);
		//i++;
	}
	client.Close();
	return 0;
}

void showMenu(){
	printf("choose what do you want to do:\n");
	printf("1.send message\n");
	printf("2.send file\n");
}