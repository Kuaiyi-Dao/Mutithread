#include "WClient.h"
#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Return_Button.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Text_Editor.H>
#include <FL/Fl_Radio_Round_Button.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Text_Display.H>
#include <FL/Fl_Widget.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Output.H>
#include <FL/Fl_File_Chooser.H>



void connectCB(Fl_Widget * w, void* arg);
void sendMsg(Fl_Widget*w, void*arg);
void sendFile(Fl_Widget*w, void*arg);

struct arg {
	Fl_Input * In1;
	Fl_Input * In2;
	Fl_Output * out;
	WinClient * Client;
};

int main()
{	
	WORD sockVersion = MAKEWORD(2, 2);
	WSADATA data;
	if (WSAStartup(sockVersion, &data) != 0)
	{
		return 0;
	}
	WinClient Client;
	Client.isConnect = false;
	Fl_Double_Window window(400, 350, "Client");					/* 1. 创建一个窗口 */
    Fl_Group* pGroup = new Fl_Group(50, 50, 400, 150);					/* 2. 创建一个分组 */
	Fl_Input * ipinput = new Fl_Input(50,50,150,30,"ip");
	Fl_Input * portinput = new Fl_Input(230, 50, 50, 30,"port");
    pGroup->end();														/* 4. 结束上个容器的创建 */
	Fl_Button * connectBtn = new Fl_Button(300,50,50,30,"连接");
	struct arg args = {ipinput,portinput,NULL,&Client};
	connectBtn->callback(connectCB,(void *)&args);
	Fl_Input * message = new Fl_Input(80, 100, 200, 30, "发送消息");
	Fl_Button * sendmsg = new Fl_Button(300, 100, 50, 30, "发送");
	Fl_Button * sendfile = new Fl_Button(150, 150, 90, 30, "发送文件");
	Fl_Output * recv = new Fl_Output(50, 200, 250, 100);
	struct arg args1 = { message,NULL,recv,&Client };
	sendmsg->callback(sendMsg, (void *)&args1);
	struct arg args2 = { NULL,NULL,NULL,&Client};
	sendfile->callback(sendFile, (void *)&args2);
    window.end();														/* 4. 结束上个容器的创建 */
    window.show();														/* 5. 显示窗口 */
    return Fl::run();													/* 6. 运行FLTK主循环 */
}

void sendFile(Fl_Widget*w, void*arg) {
	struct arg * args = (struct arg*)arg;
	if (!args->Client->isConnect)
	{
		MessageBox(NULL, "请先连接服务器", "提示", NULL);
		return;
	}
	const char* filename = fl_file_chooser("选择文件",NULL,NULL,0);
	if (filename != NULL) {
		if (!args->Client->sendMsg("2"))
		{
			args->Client->isConnect = false;
			return;
		}

		char buffer[1024];
		memset(&buffer, 0, sizeof(buffer));
		if (!args->Client->recvMsg(buffer)) {
			args->Client->isConnect = false;
			return;
		}
		memset(&buffer, 0, sizeof(buffer));
		struct FileInfo toSendFile;
		toSendFile.filename = (char*)filename;
		toSendFile.filesize = FileSize((const char*)(toSendFile.filename));
		FileMTime((const char*)(toSendFile.filename), (char*)&(toSendFile.mtime));
		if (SendFile(args->Client->client, &toSendFile))
		{
			string msg = "已发送：" + getName(filename);
			MessageBox(NULL, msg.data(), "提示", NULL);
		}
	}
}

void sendMsg(Fl_Widget*w, void*arg) {
	struct arg * args = (struct arg*)arg;
	if (!args->Client->isConnect)
	{
		MessageBox(NULL, "请先连接服务器","提示",NULL);
		return;
	}
	if (!args->Client->sendMsg("1"))
	{
		args->Client->isConnect = false;
		return;
	}
	char buffer[1024];
	memset(&buffer, 0, sizeof(buffer));
	if (!args->Client->recvMsg(buffer))
	{	
		args->Client->isConnect = false;
		return;
	}
	if (!args->Client->sendMsg(args->In1->value()))
	{
		args->Client->isConnect = false;
		return;
	}
	memset(&buffer, 0, sizeof(buffer));
	if (!args->Client->recvMsg(buffer))
	{
		args->Client->isConnect = false;
		return;
	}
	args->out->value(buffer);
	args->out->redraw();
}

void connectCB(Fl_Widget * w, void* arg) {
	struct arg * args = (struct arg*)arg;
	if (args->Client->isConnect)
		return;
	args->Client->Init(args->In1->value(), atoi(args->In2->value()));
	char buffer[1024];
	memset(&buffer, 0, sizeof(buffer));
	args->Client->recvMsg(buffer);
	if (strcmp(buffer, "accept") != 0) {
		MessageBox(NULL, "已被服务器拒绝连接！", "提示", NULL);
		return (void)0;
	}
	args->Client->isConnect = true;
	MessageBox(NULL, "已连接服务器！", "提示", NULL);
}
