#include "IDList.h"


CetID::CetID(){
	InIdlist = NULL;
	char * t = "127.0.0.1";
	//WhiteList.push_back(t);
}

int CetID::AddIp(char *_ip,int _socktfd){
	if (IsInWL(_ip)){
		idlist* ipItem = (idlist*)malloc(sizeof(idlist));
		if (ipItem == NULL){
			printf("malloc fail\n");
			return 0;
		}
		ipItem->ip = (char*)malloc(sizeof(strlen(_ip)));
		if (ipItem->ip!=NULL)
		strcpy(ipItem->ip,_ip);
		else {
			printf("malloc fail\n");
			return 0;
		}
		ipItem->socketFd = _socktfd;
		ipItem->next = NULL;
		this->InIdlist = ipItem;
		return 1;
	}else{
		printf("ip not in whitelist\n");
		return 0;
	}
	return 1;
}

int CetID::IsInWL(char* checkIp){
	for (auto itor = WhiteList.begin();itor != WhiteList.end();++itor){
		if (strcmp((*itor).data(),checkIp)==0){
			return 1;
		}
	}
	return 0;
}

int CetID::RmIp(int _socktFd){
	idlist* i = this->InIdlist;
	idlist* t = NULL;
	if (i == NULL)
	{
		printf("客户端列表为空，删除失败\n");
		return 0;
	}
	for (;i!=NULL;i = i->next){
		if (i->socketFd != _socktFd)
		{
			t = i;
		}
		else if (t == NULL)
		{
			InIdlist = InIdlist->next;
			free(i);
			return 1;
		}else{
			t->next = i->next;
			free(i);
			return 1;
		}
	}
	return 0;
}