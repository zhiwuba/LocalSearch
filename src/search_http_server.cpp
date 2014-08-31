#include <stdio.h>
#include <stdlib.h>

#include "search_http_server.h"
#include "search_query.h"


char dec_to_char(int n)
{
	if ( 0<=n&&n<=9 )
	{
		return char('0'+n);
	}
	else if ( 10<=n&&n<=15 )
	{
		return char('A'+n-10);	
	}
	else
	{
		return char(0);
	}
}

std::string url_encode(std::string source_url)
{
	std::string result;
	for ( unsigned int i=0; i<source_url.size(); i++ )
	{
		char c=source_url[i];
		if ( ('0'<=c&&c<='9')||('a'<=c&&c<='z')||('A'<=c&&c<='Z')||c=='.')
		{
			result+=c;
		}
		else
		{
			int j=(int)c;
			if ( j<0 )
			{
				j+=256;
			}
			result+='%';
			result+=dec_to_char((j>>4));
			result+=dec_to_char((j%16));
		}
	}
	return result;
}


Search_HttpServer::Search_HttpServer()
{

}

Search_HttpServer::~Search_HttpServer()
{
}

int Search_HttpServer::start_server( int port )
{
	int  result=0;
	m_listen_socket=socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if ( m_listen_socket==INVALID_SOCKET )
	{
		printf("Error: start_server socket() error.");
		return -1;
	}
	
	int flag=1;
	int size=sizeof(flag);
	if( SOCKET_ERROR==setsockopt(m_listen_socket, SOL_SOCKET, SO_REUSEADDR, (const char*)&flag, size) )
	{
		return -1;
	}

	sockaddr_in addr;
	addr.sin_family=AF_INET;
	addr.sin_addr.s_addr=inet_addr("127.0.0.1");
	addr.sin_port=htons(port);
	result=bind(m_listen_socket,(struct sockaddr*)&addr, sizeof(addr));
	if(  result==SOCKET_ERROR )
	{
		closesocket(m_listen_socket);
		return -1;
	}

	result=listen(m_listen_socket, SOMAXCONN);
	if( result==SOCKET_ERROR )
	{
		closesocket(m_listen_socket);
		return -1;
	}

	unsigned int thread_id;
	m_thread_handle=thread_create(NULL,0, (THREAD_FUN)&http_server_thread, (void*)this, 0, &thread_id );

	return 0;
}

int Search_HttpServer::stop_server()
{
	m_exit=true;
	shutdown(m_listen_socket ,SD_BOTH);
	closesocket(m_listen_socket);
	return 0;
}

int Search_HttpServer::http_server_thread_aid()
{
	while(false==m_exit)
	{
		int client_socket=accept(m_listen_socket, NULL,NULL);
		if ( INVALID_SOCKET==client_socket )
		{
			printf("Accept error in http_server_thread_aid.  %d \n", lasterror);
			continue;
		}
		
		std::string   question;
		std::string   answer;
		int       length=0;

		recv_request(client_socket, question);

		search_words(question, answer);
		
		send_response(client_socket, answer);

		closesocket(client_socket);
	}
	return 0;
}

#ifdef WIN32
unsigned int Search_HttpServer::http_server_thread(void* param)
#else
void* Search_HttpServer::http_server_thread(void* param)
#endif
{
	Search_HttpServer* pthis=(Search_HttpServer*)param;
#ifdef WIN32
	return pthis->http_server_thread_aid();
#else
	pthis->http_server_thread_aid();
	return NULL;
#endif
}

int Search_HttpServer::recv_request(int sock, std::string& question)
{
	char buffer[2048];
	int ret=recv(sock, buffer, 10240, 0);
	if( ret>0 )
	{
		char  buffer2[1024];
		char* keyword=strstr(buffer, "GET /?search=");
		char* res_end=strstr(buffer, " HTTP/1.1");
		
		int len=res_end-keyword-13;
		strncpy(buffer2, keyword+13, len);
		buffer2[len]='\0';
	
		question=url_decode(buffer2);	
	}

	return ret;
}

int Search_HttpServer::search_words(std::string quetion, std::string& answer)
{
	std::vector<std::string> answer_vec;
	g_Query.query(quetion, answer_vec);
	
	for ( int i=0; i< answer_vec.size(); i++ )
	{
		answer+=(answer_vec[i]+"<br>");
	}
	return 0;
}

int Search_HttpServer::send_response(int sock, std::string& answer)
{
	int ret=send(sock, answer.c_str(), answer.size(), 0);
	return ret>0?0:-1;
}

std::string Search_HttpServer::url_decode(std::string source_url)
{
	std::string result;
	for ( unsigned int i=0;i<source_url.size(); i++ )
	{
		char c=source_url[i];
		if ( c!='%' )
		{
			result+=c;
		}
		else
		{
			//(char_to_dec(source_url[++i])<<4) +char_to_dec(source_url[++i]); 这是错误的写法 i值相同
			int first=(char_to_dec(source_url[++i])<<4);
			int second=char_to_dec(source_url[++i]);
			int num=first+second;
			result+=(char)num;
		}
	}
	return result;
}

int Search_HttpServer::char_to_dec(char c)
{
	if ( '0'<=c&&c<='9' )
	{
		return (int)(c-'0');
	}
	else if('a'<=c&&c<='f')
	{
		return (int)(c-'a'+10);
	}
	else if('A'<=c&&c<='F')
	{
		return (int)(c-'A'+10);
	}
	else
	{
		return -1;
	}
}

