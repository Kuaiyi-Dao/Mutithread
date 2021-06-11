#include "TcpClass.h"

void showMenu();

int main(){
	TcpClient client;
	client.ConnectToServer("127.0.0.1",8086);
	char buffer[1024];
	char choice[2];
	while(true)
	{
		memset(&buffer,0,sizeof(buffer));
		//sprintf(buffer,"这是%d第%d次消息",getpid(),i+1);
		showMenu();
		scanf("%s",&choice);
		client.Write((const char*)choice);
		if (choice[0] == '1'){
			scanf("%s",&buffer);
			client.Write(buffer);
			memset(&buffer,0,sizeof(buffer));
			client.Read(buffer);
			printf("%s\n", buffer);
		}else if (choice[0] == '2'){
			memset(&buffer,0,sizeof(buffer));
			if(client.Read(buffer))
			{
				struct FileInfo sendFile;
				memset(&sendFile,0,sizeof(FileInfo));
				scanf("%s",&(sendFile.filename));
				sendFile.filesize = FileSize((const char*)&(sendFile.filename));
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