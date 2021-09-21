#include <bits/stdc++.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>
#include <mutex>
#define MAX_LEN 200
using namespace std;

class info
{
	public:
		int id;
		string name;
		int socket;
		thread th;
};

vector<info> clients;

int total_clients=0;
mutex sy_nc;

int send_to_all(string message, int sender_id);
void clientManager(int client_socket, int id);

int main()
{
	int sock_fd;
	if((sock_fd=socket(AF_INET,SOCK_STREAM,0))==-1)
	{
		perror("socket: ");
		exit(-1);
	}

	struct sockaddr_in server;
	server.sin_family=AF_INET;
	server.sin_port=htons(10004);
	server.sin_addr.s_addr=INADDR_ANY;
	bzero(&server.sin_zero,0);

	//binding the socket
	if((bind(sock_fd,(struct sockaddr *)&server,sizeof(struct sockaddr_in)))<0)
	{
		perror("bind error: ");
		exit(-1);
	}

	//listening on the socket
	if((listen(sock_fd,10))<0)
	{
		perror("listen error: ");
		exit(1);
	}

	struct sockaddr_in client;
	int client_socket;
	unsigned int len=sizeof(sockaddr_in);

	cout<<"\n\t  ====== Welcome to the chat-room ======   "<<endl
			<<"\t==== To exit chat type *exitChat* ===="<<endl;

	while(1)
	{
		if((client_socket=accept(sock_fd,(struct sockaddr *)&client,&len))<0)
		{
			perror("accept error: ");
			exit(1);
		}
		total_clients++;
		thread t(clientManager,client_socket,total_clients);

		clients.push_back({total_clients, string("Name"),client_socket,(move(t))});
	}

	for(int i=0; i<clients.size(); i++)
	{
		if(clients[i].th.joinable())
			clients[i].th.join();
	}

	close(sock_fd);
	return 0;
}




// Broadcast message to all clients except the sender
int send_to_all(string message, int sender_id)
{
	char temp[MAX_LEN];
	strcpy(temp,message.c_str());
	for(int i=0; i<clients.size(); i++)
	{
		if(clients[i].id!=sender_id)
		{
			send(clients[i].socket,temp,sizeof(temp),0);
		}
	}
    return 0;
}



void clientManager(int client_socket, int id)
{
	char name[MAX_LEN],str[MAX_LEN];
	recv(client_socket,name,sizeof(name),0);
	for(int i=0; i<clients.size(); i++) {
		if(clients[i].id==id) {
			clients[i].name=string(name);
		}
	}


	while(1)
	{
		int bytes_received=recv(client_socket,str,sizeof(str),0);
		if(bytes_received<=0)
			return;
		if(strcmp(str,"*exitChat*")==0)
		{
			// Display leaving message
			string message=string(name)+string(" has left");
			send_to_all("#NULL",id);
			send_to_all(message,id);
			cout<<message<<endl;
			for(int i=0; i<clients.size(); i++)
			{
				if(clients[i].id==id)
				{
					//lock_guard<mutex> guard(sy_nc);
					clients[i].th.detach();
					clients.erase(clients.begin()+i);
					close(clients[i].socket);
					break;
				}
			}
			return;
		}
		send_to_all(string(name),id);
		send_to_all(string(str),id);
		cout<<string(name)<<" : "<<str<<endl;
	}
}
