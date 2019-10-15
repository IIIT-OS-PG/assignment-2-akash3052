#include <unistd.h> 
#include <stdio.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include <string.h> 
#include <netdb.h>
#include <arpa/inet.h>
#include<bits/stdc++.h>
#include <iostream>
#include <cstdlib>
#include <pthread.h>

// #define chunk_size 

#define trackerport1 50005
using namespace std;

map<string,vector<int>> m;
vector<int> v;
 
struct  client_upload
{
	string type;
	string trackerip;
	string trackerport;
	string ownport;
	string ownip;
	string filename;
	string filesize;
	string path;

};

struct client_download
{
	string port;
	string ip;
	string readpath,writepath;
	int chunk_number;
};

struct  server_d
{
	string port;
};

struct  serveclient
{
	int i;
};

struct send_data_to_read
{
	int i;
	int chunk_number;
	// string readpath;
	FILE *fp;
};

struct  send_data_to_write//clientdownload
{
	int i;
	int chunk_number;
	FILE *fp;
};

void* write1(void *threadarg)
{
	struct send_data_to_write *c;
	c=(struct send_data_to_write*)threadarg;
	int newsockfd=c->i;
	int chunk_number=c->chunk_number;
	FILE *fp=c->fp;
	// FILE *fp;
	// // fp=fopen("test_servr.txt","wb");
	// fp=fopen("/home/ak/Desktop/2019201046/server.txt","wb");
	int filesize=512;
	
	// recv(newsockfd,&filesize,sizeof(int),0);

	// cout<<"filesize :"<<filesize<<endl;
	int n;
	char buff[512];
	memset(buff,'\0',512);//added new
	// rewind(fp);
	fseek(fp,0,SEEK_SET);
	fseek(fp,chunk_number*512,SEEK_SET);
	cout<<"ftell in write1 before writing :"<<ftell(fp)<<endl;
	n=recv(newsockfd,buff,512,0);
	// while((n=recv(newsockfd,buff,10,0))>0 && filesize>0)
	// {
	cout<<"bytes received in write from server (value of n):"<<n<<endl;
		fwrite(buff,sizeof(char),n,fp);
		// bzero(buff,1024);cout<<"ftell in write1:"<<ftell(fp)<<endl;
		cout<<"ftell in write1 after writing :"<<ftell(fp)<<endl;
		memset(buff,'\0',512);
		filesize=filesize-512;
	// }
	

	// v[chunk_number]=1;
	//m[filename]=v;
	// fclose(fp);
	pthread_exit(NULL);
}


void* read1(void *threadarg)
{
	struct send_data_to_read *s;
	s=(struct send_data_to_read*)threadarg;
	int sockfd=s->i;
	int chunk_number=s->chunk_number;
	FILE *fp=s->fp;;
	
	// fp=fopen("/home/ak/Desktop/upload.pdf","rb");

	
	// cout<<"size : "<<size<<endl;
	fseek(fp,0,SEEK_SET);
	fseek(fp,chunk_number*512,SEEK_SET);
	cout<<"\nftell in read1 :"<<ftell(fp)<<endl;
	// send(sockfd,&size,sizeof(size),0);
	char buffer[512];
	memset(buffer,'\0',512);
	int n;
	int size=512;
	n=fread(buffer,sizeof(char),512,fp);
	// while((n=fread(buffer,sizeof(char),512,fp))>0 && size>0)
	// {
		send(sockfd,buffer,n,0);
		// bzero(buffer,1024);
		memset(buffer,'\0',512);
		// size=size-512;
		cout<<"value of ftell in read inside while loop :"<<ftell(fp)<<endl;
	// }
	cout<<"value of ftell in read outside while loop :"<<ftell(fp)<<endl;
	// fclose(fp);
	pthread_exit(NULL);
}


void* server(void *threadarg)
{
	struct server_d *mydata;
	mydata=(struct server_d*)threadarg;
	string port1=mydata->port;
	// cout<<port1<<endl;

	int sockfd,newsockfd;
	struct sockaddr_in myaddress,client_addr;
	sockfd=socket(AF_INET,SOCK_STREAM,0);
	// int port=atoi(argv[1]);
	int port=atoi(port1.c_str());
	if(sockfd<0)
	{
		cout<<"error in socket creation<<endl";
		// return -1;
		// return;
	}

	myaddress.sin_family=AF_INET;
	myaddress.sin_port=htons(port);
	myaddress.sin_addr.s_addr=INADDR_ANY;

	int i=bind(sockfd,(struct sockaddr*)&myaddress,sizeof(myaddress));
	if(i<0)
		cout<<"error in binding.."<<endl;

	if(listen(sockfd,5)<0)
	{
		cout<<"listen error.."<<endl;
		// return -1;
		// return;
	}
	cout<<"server started with port no "<<port<<endl;

	socklen_t len=(sizeof(client_addr));
	
	while(1)
	{
		if((newsockfd=accept(sockfd,(struct sockaddr*)&client_addr,&len))<0)
		{
			cout<<"error in new socket formation"<<endl;
				// return -1;
			// return;
		}
		/*************writing this code to transfer the chunk informaiton **************/

		int a;
		recv(newsockfd,&a,sizeof(int),0);

		cout<<"\nreceived value of a in server :"<<a<<endl;
		if(a==1)
		{
			char filename[500];
			recv(newsockfd,filename,sizeof(filename),0);
			cout<<"filename received :"<<filename<<endl;

			vector<int> vector_to_send;
			vector_to_send =m[filename];

			int size=vector_to_send.size();
			send(newsockfd,&size,sizeof(int),0);

			int arr[size];
			for(int i=0;i<size;i++)
				arr[i]=vector_to_send[i];

			send(newsockfd,arr,size*sizeof(int),0);
		}

		//**************(a==2) this is to read data from file and send it to the client********************************

		if(a==2)
		{


		int chunk;
		recv(newsockfd,&chunk,sizeof(int),0);

		char readpath[500];
		recv(newsockfd,readpath,sizeof(readpath),0);
		cout<<"\npath received from client to read file :"<<readpath<<endl;

		FILE *fp;
		// fp=fopen("test_servr.txt","wb");
		fp=fopen(readpath,"r");

		cout<<"\nvalue of chunk in server :"<<chunk<<endl;
		struct send_data_to_read detail;
		detail.i=newsockfd;
		detail.chunk_number=chunk;
		detail.fp=fp;
		// detail.readpath=readpath;
		pthread_t tid;
		pthread_create(&tid,NULL,read1,(void*)&detail);
		pthread_join(tid,NULL);
		fclose(fp);

		}//ending else part

		close(newsockfd);

	}
	
	close(sockfd);

	pthread_exit(NULL);
	//return 0;
}

void* downloadfunc(void *threadarg)
{
	//get ip and port of all the clients which have the file
	//connect to all clients and get all the chunks they have
	//design algo so that no client will sit idle 
	struct client_download *mydata;
	mydata=(struct client_download*)threadarg;

	string ipadd=mydata->ip;
	string port1=mydata->port;
	int chunk_number=mydata->chunk_number;
	string readpath=mydata->readpath;
	string writepath=mydata->writepath;

	int sockfd;
	struct sockaddr_in serveraddr;
	struct hostent *server;
	sockfd=socket(AF_INET,SOCK_STREAM,0);
	// int port=atoi(argv[2]);
	int port=atoi(port1.c_str());

	if(sockfd<0)
	{
		cout<<"error in socket creation..\n";
		// return -1;
		// return;
	}

	bzero((char *)&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family=AF_INET;
	serveraddr.sin_port=htons(port);
	// server=gethostbyname(argv[1]);
	server=gethostbyname(ipadd.c_str());

	bcopy((char*)server->h_addr,(char*)&serveraddr.sin_addr.s_addr,server->h_length);

	
	if(connect(sockfd,(struct sockaddr *)&serveraddr,sizeof(serveraddr))<0)
	{
		cout<<"connetion failed"<<endl;
		// return -1;
		// return;
	}


	int a=2;
	send(sockfd,&a,sizeof(int),0);

	cout<<"chunk number in client :"<<chunk_number<<endl;
	send(sockfd,&chunk_number,sizeof(int),0);

	char buffer_readpath[500];
	strcpy(buffer_readpath,readpath.c_str());
	send(sockfd,buffer_readpath,sizeof(buffer_readpath),0);

	FILE *fp;
	// fp=fopen("test_servr.txt","wb");
	fp=fopen(writepath.c_str(),"r+");//r+ used for both write and update

	struct send_data_to_write detail;
	detail.i=sockfd;
	detail.chunk_number=chunk_number;
	detail.fp=fp;
	pthread_t tid;
	pthread_create(&tid,NULL,write1,(void*)&detail);
	pthread_join(tid,NULL);

	fclose(fp);
	close(sockfd);
	
	pthread_exit(NULL);

}

void* uploadfunc(void *threadarg)
{
	struct client_upload *mydata;
	mydata=(struct client_upload*)threadarg;
	string trackerport=mydata->trackerport;
	string trackerip=mydata->trackerip;
	string ownip=mydata->ownip;
	string ownport=mydata->ownport;
	string filename=mydata->filename;
	string path=mydata->path;
	string size=mydata->filesize;
	// string type=mydata->type;

	string data_to_send=filename+"$"+ownip+" "+ownport+" "+path+" "+size;
	cout<<"string data_to_send :"<<data_to_send<<endl;
	
	cout<<"\ntrackerport :"<<trackerport<<"\ntrackerip :"<<trackerip<<endl;
	cout<<"\nfilename in uploadfunc:"<<filename<<"\npath value in uploadfunc:"<<path<<endl;

	int sockfd;
	struct sockaddr_in serveraddr;
	struct hostent *server;
	sockfd=socket(AF_INET,SOCK_STREAM,0);
	
	int port=atoi(trackerport.c_str());
	// int port=atoi(ownport.c_str());
	if(sockfd<0)
	{
		cout<<"error in socket creation..\n";
		// return -1;
		// return;
	}

	bzero((char *)&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family=AF_INET;
	serveraddr.sin_port=htons(port);
	server=gethostbyname(trackerip.c_str());
	// server=gethostbyname(ownip.c_str());

	bcopy((char*)server->h_addr,(char*)&serveraddr.sin_addr.s_addr,server->h_length);


	if(connect(sockfd,(struct sockaddr *)&serveraddr,sizeof(serveraddr))<0)
	{
		cout<<"connetion failed"<<endl;
		// return -1;
		// return;
	}
	int a=1;
	send(sockfd,&a,sizeof(int),0);
	char buffer[500];
	strcpy(buffer,data_to_send.c_str());
	cout<<"value of data_to_send in peer :"<<buffer<<endl;
	send(sockfd,buffer,sizeof(buffer),0);
	close(sockfd);
	// send(sockfd,mydata,sizeof(struct  client_upload),0);
	pthread_exit(NULL);
	// return 0;
}




int main(int argc ,char *argv[])
{
	pthread_t serv_id;
	int status_s;

	struct server_d server_data;
	server_data.port=argv[2];

	status_s=pthread_create(&serv_id,NULL,server,(void*)&server_data);
	if(status_s!=0)
	{
		cout<<"error in thread creation for server "<<endl;
		exit(-1);
	}
	pthread_detach(serv_id);

	while(1)
	{

	cout<<"1.Upload\n2.Download\n3.Downloads by communicating with tracker\n";
	int s;
	cout<<"enter choice :"<<endl;
	cin>>s;
	if(s==1)
	{
		string filename,path;
		cout<<"enter filename :";
		cin>>filename;
		cout<<"enter path :";
		cin>>path;

		pthread_t tid;
		int status;
		
		FILE *fp;
		fp=fopen(path.c_str(),"rb");
		fseek(fp,0,SEEK_END);
		int size1=ftell(fp);
		string size=to_string(size1);
		cout<<"size"<<size<<endl;
		fclose(fp);

		/*bit vector to keep track of which chunk this client has.*/
		int no_of_chunks=size1/512;
		if(size1%512!=0)
			no_of_chunks++;

		// vector<int> v(no_of_chunks);
		for(int i=0;i<no_of_chunks;i++)
			v.push_back(1);
		m[filename]=v;


		//To check the value of bit vector in client

			// cout<<"\nvalues in map in client with port no :"<<argv[2]<<endl;
			// for(auto it=m.begin();it!=m.end();it++)
			// {
			// 	cout<<"filename :"<<it->first<<endl;
			// 	cout<<"details :"<<endl;
			// 	vector<int> v=it->second;
			// 	for(auto x:v)
			// 		cout<<x;
			
			// }

		//assigning value to client
		struct client_upload c;
		c.type="upload";
		c.trackerip="127.0.0.1";
		c.trackerport="50005";
		c.ownip=argv[1];
		c.ownport=argv[2];
		c.filename=filename;
		c.path=path;
		c.filesize=size;

		status=pthread_create(&tid,NULL,uploadfunc,(void*)&c);
		pthread_join(tid,NULL);

	}

	if(s==2)
	{
		string filename,readpath,writepath,ip;
		cout<<"enter filename :"<<endl;
		cin>>filename;
		cout<<"enter read path :"<<endl;
		cin>>readpath;
		cout<<"enter write path :"<<endl;
		cin>>writepath;
		cout<<"enter ip address :"<<endl;
		cin>>ip;


		// fp=fopen(path.c_str(),"rb");

		FILE *fp1;
	
		fp1=fopen(readpath.c_str(),"rb");
		fseek(fp1,0,SEEK_END);
		long long filesize=ftell(fp1);
		cout<<"reading filesize :"<<filesize;
		fclose(fp1);


		FILE *fp;
		fp=fopen(writepath.c_str(),"w");
		fclose(fp);


		int no_of_chunks=filesize/512;
		if(filesize%512!=0)
			no_of_chunks++;
		for(int i=0;i<no_of_chunks;i++)
		{

			// for(int j=1;j<=3;j++)
			// {


				cout<<"value of i :"<<i<<endl;
				string port;
			// cout<<"enter port :"<<endl;
			// cin>>port;

				if(i%3==0)
					port="50001";
				else if(i%3==1)
					port="50002";
				else if(i%3==2)
					port="50003";

				struct client_download c;
				c.port=port;
				c.chunk_number=i;
				c.readpath=readpath;
				c.writepath=writepath;
				c.ip=ip;

				cout<<"\nclient number :"<<port<<"chunk_number :"<<i<<endl;

				pthread_t tid;
				int status;
				status=pthread_create(&tid,NULL,downloadfunc,(void*)&c);
				pthread_join(tid,NULL);
			// }

		}//for outer loop
		
	}//this brace is for (if(s==2))


	if(s==3)
	{

		string filename,path;
		cout<<"enter filename :"<<endl;
		cin>>filename;
		cout<<"enter download path :"<<endl;
		cin>>path;

		//code to make connection with tracker to get data all the clients where file is present
		int sockfd;
		string trackerip="127.0.0.1";
		struct sockaddr_in serveraddr;
		struct hostent *server;
		sockfd=socket(AF_INET,SOCK_STREAM,0);
	
		int port=trackerport1;
		// int port=atoi(ownport.c_str());
		if(sockfd<0)
		{
			cout<<"error in socket creation..\n";
			// return -1;
			// return;
		}

		bzero((char *)&serveraddr, sizeof(serveraddr));
		serveraddr.sin_family=AF_INET;
		serveraddr.sin_port=htons(port);
		server=gethostbyname(trackerip.c_str());
		// server=gethostbyname(ownip.c_str());

		bcopy((char*)server->h_addr,(char*)&serveraddr.sin_addr.s_addr,server->h_length);


		if(connect(sockfd,(struct sockaddr *)&serveraddr,sizeof(serveraddr))<0)
		{
			cout<<"connetion failed"<<endl;
			// return -1;
			// return;
		}
		int a=2;
		send(sockfd,&a,sizeof(int),0);

		char buffer[500];
		strcpy(buffer,filename.c_str());
		cout<<"value of data_to_send in peer :"<<buffer<<endl;
		send(sockfd,buffer,sizeof(buffer),0);

		//receving data from tracker

		char recv_buffer[500];
		recv(sockfd,recv_buffer,sizeof(recv_buffer),0);

	
		cout<<"received data in buffer :"<<recv_buffer<<endl;

		close(sockfd);

		/************************breaking data for each client**************/

		vector<string> v1;
		char *ptr=strtok(recv_buffer,"$");
		while(ptr!=NULL)
		{
			string s1(ptr);
			v1.push_back(s1);
			cout<<ptr<<endl;
			ptr=strtok(NULL,"$");
		}


		int no_of_clients=v1.size();
		cout<<"no of client :"<<no_of_clients<<endl;
		for(auto x:v1)
			cout<<"Client information: "<<x<<endl;

		vector<string> client_info[no_of_clients];


		/***************getting ip and port of each client in client_info****************/
		for(int i=0;i<no_of_clients;i++)
		{
			string str1=v1[i];
			char buffer3[500];
			strcpy(buffer3,str1.c_str());
			char *ptr2=strtok(buffer3," ");
			while(ptr2!=NULL)
			{
				string s2(ptr2);
				client_info[i].push_back(s2);
				ptr2=strtok(NULL," ");
			}
		}

		for(int i=0;i<no_of_clients;i++)
		{
			for(auto x:client_info[i])
				cout<<x<<endl;
		}

		/***************make connection with each client to get chunk information and storing it in vector chunk_info****/

		vector<int> chunk_info[no_of_clients];

		string filesz=client_info[0][3];
		int filesize=atoi(filesz.c_str());

		cout<<"filesize received from tracker :"<<filesize<<endl;

		int no_of_chunks=filesize/512;
		if(filesize%512!=0)
			no_of_chunks++;
		//*************************************************************************************************
		for(int i=0;i<no_of_clients;i++)
		{
			int sockfd;
			struct sockaddr_in serveraddr;
			struct hostent *server;
			sockfd=socket(AF_INET,SOCK_STREAM,0);
			// int port=atoi(argv[2]);
			// int port=50002;
			string port_str=client_info[i][1];
			int port=atoi(port_str.c_str());
			string ipadd=client_info[i][0];
			if(sockfd<0)
			{
				cout<<"error in socket creation..\n";
				// return -1;
				// return;
			}
		

			bzero((char *)&serveraddr, sizeof(serveraddr));
			serveraddr.sin_family=AF_INET;
			serveraddr.sin_port=htons(port);
			// server=gethostbyname(argv[1]);
			server=gethostbyname(ipadd.c_str());

			bcopy((char*)server->h_addr,(char*)&serveraddr.sin_addr.s_addr,server->h_length);

	
			if(connect(sockfd,(struct sockaddr *)&serveraddr,sizeof(serveraddr))<0)
			{
				cout<<"connetion failed"<<endl;
				// return -1;
				// return;
			}

			int a=1;
			send(sockfd,&a,sizeof(int),0);

			// string str1="akash.txt";
			// string str1=client_info[i][];
			char filename1[100];
			strcpy(filename1,filename.c_str());
			send(sockfd,filename1,sizeof(filename1),0);

			int size;
			recv(sockfd,&size,sizeof(int),0);
			cout<<"\nsize received :"<<size<<endl;
	
			int arr[size];

			recv(sockfd,arr,size*sizeof(int),0);
			cout<<"values after receiving data from server :"<<endl;
			for(int i=0;i<size;i++)
			cout<<arr[i];

			// vector<int> v(size);
			// for(int i=0;i<size;i++)
			// 	v[i]=arr[i];

			for(int j=0;j<no_of_chunks;j++)
				chunk_info[i].push_back(arr[j]);
	
	
		}//for loop ends here

		//************************************************************************************
		cout<<"printing chunck information.."<<endl;
		for(int i=0;i<no_of_clients;i++)
		{
			for(int j=0;j<no_of_chunks;j++)
			{
				cout<<chunk_info[i][j];
			}
			cout<<endl;
		}	

		/**************applying peer selection algorithm *****************/
		int client_number,chunk_index;

		FILE *fp;
		fp=fopen(path.c_str(),"w");
		fclose(fp);


		for(int i=0;i<no_of_chunks;i++)
		{
			for(int j=(i%no_of_clients);j<no_of_clients;i++)
			{
				if(chunk_info[j][i]==1)
				{
					client_number=j;
					break;
				}
			}
			chunk_index=i;

			string port_number=client_info[client_number][1];

			/*******************copying from hardcoded download ***************************/

				// FILE *fp;
				// fp=fopen("/home/ak/Desktop/2019201046/server.pdf","w");
				// fclose(fp);
				
				// string readpath,writepath;

				struct client_download c;
				c.port=port_number;
				c.chunk_number=chunk_index;
				c.readpath=client_info[client_number][2];
				c.writepath=path;
				c.ip=client_info[client_number][0];//ip address added

				// cout<<"\nclient number :"<<port_number<<"  chunk_number :"<<chunk_index<<"  read path :";
				// cout<<client_info[client_number][2]<<"  write path :"<<path<<endl;

				pthread_t tid;
				int status;
				status=pthread_create(&tid,NULL,downloadfunc,(void*)&c);
				pthread_join(tid,NULL);
			


				/***************************************/


		}//this brace is of for loop (number of chunks)


		
	}//this brace is for s=3
	
	

	}//this brace is for while loop
	pthread_exit(NULL);

	return 0;
}