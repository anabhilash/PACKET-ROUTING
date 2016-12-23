#include "stdafx.h"
#include <winsock2.h>
#include <windows.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>

#include<conio.h>
int readdigits(char *);
char** readFile(char *, int *, int *, int *, int *, int *, int *);
int readNum(char *);
void path_exist(char **, int, int, int, int, int, int, int *,char *);
int v = 0;
int x = 0;
struct node
{
	char msg[128];
	int msg_id;
	node *next;
}*flist,*alist,*printid;

struct bufserv{
	
		int userId;
		int forumId;
		int msgId;
		int commentId;
		int choice;
		char *forumname;
		char msg[128];
}buf1;

bool flag=true;
int mid = 0;
int count1 =0;
char *Data[100];
int count=1;
int values[100];
DWORD WINAPI SocketHandler(void*);
void replyto_client(char *buf, int *csock);

void socket_server() {

	//The port you want the server to listen on
	int host_port= 1101;

	unsigned short wVersionRequested;
	WSADATA wsaData;
	int err;
	wVersionRequested = MAKEWORD( 2, 2 );
 	err = WSAStartup( wVersionRequested, &wsaData );
	if ( err != 0 || ( LOBYTE( wsaData.wVersion ) != 2 ||
		    HIBYTE( wsaData.wVersion ) != 2 )) {
	    fprintf(stderr, "No sock dll %d\n",WSAGetLastError());
		goto FINISH;
	}

	//Initialize sockets and set options
	int hsock;
	int * p_int ;
	hsock = socket(AF_INET, SOCK_STREAM, 0);
	if(hsock == -1){
		printf("Error initializing socket %d\n",WSAGetLastError());
		goto FINISH;
	}
	
	p_int = (int*)malloc(sizeof(int));
	*p_int = 1;
	if( (setsockopt(hsock, SOL_SOCKET, SO_REUSEADDR, (char*)p_int, sizeof(int)) == -1 )||
		(setsockopt(hsock, SOL_SOCKET, SO_KEEPALIVE, (char*)p_int, sizeof(int)) == -1 ) ){
		printf("Error setting options %d\n", WSAGetLastError());
		free(p_int);
		goto FINISH;
	}
	free(p_int);

	//Bind and listen
	struct sockaddr_in my_addr;
	my_addr.sin_family = AF_INET ;
	my_addr.sin_port = htons(host_port);
	
	memset(&(my_addr.sin_zero), 0, 8);
	my_addr.sin_addr.s_addr = INADDR_ANY ;
	
	/* if you get error in bind 
	make sure nothing else is listening on that port */
	if( bind( hsock, (struct sockaddr*)&my_addr, sizeof(my_addr)) == -1 ){
		fprintf(stderr,"Error binding to socket %d\n",WSAGetLastError());
		goto FINISH;
	}
	if(listen( hsock, 10) == -1 ){
		fprintf(stderr, "Error listening %d\n",WSAGetLastError());
		goto FINISH;
	}
	
	//Now lets do the actual server stuff

	int* csock;
	sockaddr_in sadr;
	int	addr_size = sizeof(SOCKADDR);
	
	while(true){
		printf("waiting for a connection\n");
		csock = (int*)malloc(sizeof(int));
		
		if((*csock = accept( hsock, (SOCKADDR*)&sadr, &addr_size))!= INVALID_SOCKET ){
			//printf("Received connection from %s",inet_ntoa(sadr.sin_addr));
			CreateThread(0,0,&SocketHandler, (void*)csock , 0,0);
		}
		else{
			fprintf(stderr, "Error accepting %d\n",WSAGetLastError());
		}
	}

FINISH:
;
}


void process_input(char *recvbuf, int recv_buf_cnt, int* csock) 
{

	char replybuf[1024]={'\0'};
	printf("%s",recvbuf);
	char **a = NULL;
	int s1, s2;
	int d1, d2;
	int r, c;
	a = readFile(recvbuf, &r, &c, &s1, &s2, &d1, &d2);
	int path = 0;
	memset(replybuf, 0, 1024);
	if ((((s1 >= 0) && (s1 < r)) && (s2 >= 0) && (s2 < c)) && (((d1 >= 0) && (d1 < r)) && ((d2 >= 0) && (d2 < r))))
	{
		if ((a[s1][s2] == '1') && (a[d1][d2] == '1'))
		{
			path_exist(a, s1, s2, d1, d2, r, c, &path,replybuf);
		}
	}

	

	if (path == 0)
	{
		replybuf[0] = EOF;
	}
	else
	{
		replybuf[x] = '\0';
		replyto_client(replybuf, csock);
	}
}

void replyto_client(char *buf, int *csock) {
	int bytecount;
	
	if((bytecount = send(*csock, buf, strlen(buf), 0))==SOCKET_ERROR){
		fprintf(stderr, "Error sending data %d\n", WSAGetLastError());
		free (csock);
	}
	
}

DWORD WINAPI SocketHandler(void* lp){
    int *csock = (int*)lp;

	char recvbuf[1024];
	int recvbuf_len = 1024;
	int recv_byte_cnt;

	memset(recvbuf, 0, recvbuf_len);
	if((recv_byte_cnt = recv(*csock, recvbuf, recvbuf_len, 0))==SOCKET_ERROR){
		fprintf(stderr, "Error receiving data %d\n", WSAGetLastError());
		free (csock);
		return 0;
	}

	//printf("Received bytes %d\nReceived string \"%s\"\n", recv_byte_cnt, recvbuf);
	process_input(recvbuf, recv_byte_cnt, csock);

    return 0;
}






int readdigits(char * s)
{
	char ch;
	int num = 0;
	ch = s[v];
	v++;
	while ((ch != EOF) && ch != '\n')
	{
		if ((ch >= '0') && (ch <= '9'))
		{
			num = num * 10 + (ch - '0');
		}
		else
		{
			break;
		}
		ch = s[v];
			v++;
	}
	if (ch == ',' || ch == '\n' || ch == EOF)
	{
		return num;
	}
	else
	{
		return -1;
	}

}


int readNum(char *s)
{
	unsigned int num = 0;
	char ch;
	ch = s[v];
	v++;
	while ((ch != EOF) && ch != '\n')
	{
		if ((ch >= '0') && (ch <= '9'))
		{
			num = num * 10 + (ch - '0');
		}
		else
		{
			break;
		}
		ch = s[v];
		v++;
	}
	if (ch == '\n')
	{
		return num;
	}
	else
	{
		return -1;
	}
}


char ** readFile(char *fp, int *r, int *c, int *s1, int *s2, int *d1, int *d2)
{
	int i = 0, j = 0;
	char ch;
	*r = readNum(fp);
	if ((*r) < 0)
	{
		printf("error in rows");
		exit(1);
	}
	*c = readNum(fp);
	if ((*c) < 0)
	{
		printf("error in rows");
		exit(1);
	}
	char **a;
	a = (char **)malloc((*r)*sizeof(char *));
	for (i = 0; i < *r; i++)
	{
		a[i] = (char *)malloc((*c)*sizeof(char));
	}

	for (i = 0; i < (*r); i++)
	{
		for (j = 0; j < (*c); j++)
		{
			ch = fp[v];
			v++;//fgetc(fp);
			if (ch == '0' || ch == '1')
			{

				a[i][j] = ch;

			}
			else if (ch == ',')
			{
				j--;
			}
			else
			{
				printf("Error reading row %d", i);
				exit(1);
			}

		}
		ch = fp[v];
		v++;
		//fgetc(fp);
		if ((ch == EOF) && (i == ((*r) - 1)))
		{
			break;
		}
		if (ch != '\n')
		{
			printf("Error reading row %d", i);
			exit(1);
		}
	}
	*s1 = readdigits(fp);
	*s2 = readdigits(fp);
	*d1 = readdigits(fp);
	*d2 = readdigits(fp);
	return a;
}

void path_exist(char **a, int s1, int s2, int d1, int d2, int r, int c, int *path, char *rep)
{
	if ((s1 == d1) && (s2 == d2))
	{
		printf("%d%d\n", d1, d2);
		*path = 1;
	}
	else
	{
		//up
		if ((d1 - 1 >= 0) && (a[d1 - 1][d2] == '1') && (!(*path)))
		{
			a[d1][d2] = '0';
			path_exist(a, s1, s2, d1 - 1, d2, r, c, path,rep);
			a[d1][d2] = '1';
			if (*path)
			{
				rep[x] = d1 + '0';
				x++;
				rep[x] = ',';
				x++;
				rep[x] = d2 + '0';
				x++;
				rep[x] = '\n';
				x++;
				printf("%d%d\n", d1, d2);
			}
		}
		//left

		if ((d2 - 1 >= 0) && (a[d1][d2 - 1] == '1') && (!(*path)))
		{
			a[d1][d2] = '0';
			path_exist(a, s1, s2, d1, d2 - 1, r, c, path,rep);
			a[d1][d2] = '1';
			if (*path)
			{
				rep[x] = d1 + '0';
				x++;
				rep[x] = ',';
				x++;
				rep[x] = d2 + '0';
				x++;
				rep[x] = '\n';
				x++;
				printf("%d%d\n", d1, d2);
			}
		}

		//right

		if ((d2 + 1<c) && (a[d1][d2 + 1] == '1') && (!(*path)))
		{
			a[d1][d2] = '0';
			path_exist(a, s1, s2, d1, d2 + 1, r, c, path,rep);
			a[d1][d2] = '1';
			if (*path)
			{
				rep[x] = d1 + '0';
				x++;
				rep[x] = ',';
				x++;
				rep[x] = d2 + '0';
				x++;
				rep[x] = '\n';
				x++;
				printf("%d%d\n", d1, d2);
			}
		}

		//down

		if ((d1 + 1<r) && (a[d1 + 1][d2] == '1') && (!(*path)))
		{
			a[d1][d2] = '0';
			path_exist(a, s1, s2, d1 + 1, d2, r, c, path,rep);
			a[d1][d2] = '1';
			if (*path)
			{
				rep[x] = d1 + '0';
				x++;
				rep[x] = ',';
				x++;
				rep[x] = d2 + '0';
				x++;
				rep[x] = '\n';
				x++;
				printf("%d%d\n", d1, d2);
			}
		}

	}
}