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

#define port 50005

using namespace std;

map<string,vector<string>> m;

void* server()
{
	
	int sockfd,newsockfd;
	struct sockaddr_in myaddress,client_addr;
	sockfd=socket(AF_INET,SOCK_STREAM,0);
	// int port=atoi(argv[1]);
	// int port=50005;
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
	{
		cout<<"error in binding.."<<endl;
		exit(-1);
	}

	if(listen(sockfd,5)<0)
	{
		cout<<"listen error.."<<endl;
		// return -1;
		// return;
	}
	cout<<"Tracker server started with port no "<<port<<endl;

	socklen_t len=(sizeof(client_addr));
	
	while(1)
	{
		if((newsockfd=accept(sockfd,(struct sockaddr*)&client_addr,&len))<0)
		{
			cout<<"error in new socket formation"<<endl;
				// return -1;
			// return;
		}
		
		int a;
		recv(newsockfd,&a,sizeof(int),0);

		cout<<"received value of a :"<<a<<endl;

		cout<<"\nnew connection established.."<<endl;


		//if a is 1 that means we need to upload 
		if(a==1)
		{
			char buffer[500];
			recv(newsockfd,buffer,sizeof(buffer),0);
			// cout<<"after recv"<<endl;
			// cout<<"value of data received in tracker :"<<buffer<<endl;

			string s(buffer);
			// cout<<s<<endl;
			// cout<<"after tokeninsing :"<<endl;
			vector<string> v;
			char *ptr=strtok(buffer,"$");


			while(ptr!=NULL)
			{
				string s1(ptr);
				v.push_back(s1);
				// cout<<ptr<<endl;
				ptr=strtok(NULL,"$");
			}

		
			string filename=v[0];
			string details=v[1];
		
			cout<<"filename :"<<filename<<"\ndetails :"<<details<<endl;
			vector<string> v1;
			char buffer1[500];
			strcpy(buffer1,details.c_str());

			char *ptr1=strtok(buffer1," ");
			while(ptr1!=NULL)
			{
				string s2(ptr1);
				v1.push_back(s2);
				// cout<<ptr1<<endl;
				ptr1=strtok(NULL," ");
			}
			
			// cout<<"\nvalues in vector string "<<endl;
			// for(auto x:v1)
			// 	cout<<x<<endl;

		
			if(m.find(filename)!=m.end())
			{
				for(auto it=m.begin();it!=m.end();it++)
				{
					if(filename==it->first)
					{
						vector<string> v2=it->second;
						v2.push_back("$");
						for(int i=0;i<v1.size();i++)
						{
							v2.push_back(v1[i]);
						}
						m[filename]=v2;
					}
				}

			}
			else
			{
				m[filename]=v1;
			}		

			/*code below is to add new ipaddress and port which has the same filename*/

			// for(auto it=m.begin();it!=m.end();it++)
			// {
			// 	cout<<"filename :"<<it->first<<endl;
			// 	cout<<"details :"<<endl;
			// 	vector<string> v=it->second;
			// 	// for(auto x:v)
			// 	// 	cout<<x<<" "<<endl;
			// 	v.push_back("$");
			// 	v.push_back("127.0.0.2");
			// 	v.push_back("50006");
			// 	m[filename]=v;
			// }
			
			cout<<"values in map :"<<endl;
			for(auto it=m.begin();it!=m.end();it++)
			{
				cout<<"filename :"<<it->first<<endl;
				cout<<"details :"<<endl;
				vector<string> v=it->second;
				for(auto x:v)
					cout<<x<<" "<<endl;
			
			}
		}//braces for a=1.

		if(a==2)//downloading
		{
			//for download
			char buffer[500];
			recv(newsockfd,buffer,sizeof(buffer),0);
			cout<<"after recv"<<endl;
			cout<<"value of data received in tracker :"<<buffer<<endl;

			string filename(buffer);
			
			vector<string> v2;
			if(m.find(filename)!=m.end())
			{
				for(auto it=m.begin();it!=m.end();it++)
				{
					if(filename==it->first)
					{
						v2=it->second;
						break;
						
					}
				}

			}
			cout<<"\nwhile downloading.."<<endl;
			string str;
			for(int i=0;i<v2.size();i++)
			{
				str+=v2[i];
				if((i+1)<=v2.size()-1)
				{
					if(v2[i]=="$"|| v2[i+1]=="$")
					{
						continue;
					}
					else
					{

						str+=" ";
					}
				}
				
				cout<<str<<endl;
			}
			cout<<"str value :"<<str<<endl;
			char buffer2[500];
			strcpy(buffer2,str.c_str());

			send(newsockfd,buffer2,sizeof(buffer2),0);

		}//braces for a=2(download)

		
		close(newsockfd);

	}//this brace is for while loop
	
	close(sockfd);

}

int main(int agrc,char* argv[])
{
	server();
	return 0;
}