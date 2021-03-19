#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <cstdio>
#include <unistd.h>

using namespace std;

void create_clientSocket()
{
	int socketDesc = socket(AF_INET, SOCK_STREAM, 0);
	char response[512];

	if(socketDesc==-1)
		cout<<"\nERROR: Failed to create socket";

	struct sockaddr_in server;
    
	server.sin_addr.s_addr=inet_addr("127.0.0.1");
	server.sin_family=AF_INET;
	server.sin_port=htons(9005);

	if(connect(socketDesc, (struct sockaddr *) &server, sizeof(server)))
	{
		perror("connect");
		return;
	}

	cout<<"\nCONNECTED SUCCESSFULLY";
	recv(socketDesc, response, sizeof(response), 0);
	cout<<"\nResponse: "<<response;
	close(socketDesc);
	return;
}

int main()
{
    create_clientSocket();

    return 0;
}