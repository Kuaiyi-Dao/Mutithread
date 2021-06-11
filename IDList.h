#ifndef __IDLIST_H
#define __IDLIST_H 1
#include <cstring>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <vector>

typedef struct Idlist{
	char *ip;
	int socketFd;
	struct Idlist* next;
}idlist;

class CetID{
private:
		idlist*InIdlist;
		std::vector<std::string> WhiteList;

public:
		CetID();
		int AddIp(char* _ip,int _socktfd);
		int IsInWL(char* checkIp);
		int RmIp(int _socktFd);
};






#endif
 