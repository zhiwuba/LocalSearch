#ifndef  __SEARCH_HTTP_SERVER_H__
#define __SEARCH_HTTP_SERVER_H__

#include <string>
#ifdef WIN32
#include <process.h>
#include <Winsock2.h>
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/time.h>
#include <unistd.h>

#define SOCKET int
#define SOCKET_ERROR -1
#define INVALID_SOCKET -1
#define SD_BOTH SHUT_RDWR  
#define IPPROTO_TCP  0

#define closesocket close
#endif // WIN32

#define  g_HttpServer Search_HttpServer::instance()

class Search_HttpServer
{
public: 
	static Search_HttpServer instance()
	{
		static Search_HttpServer _instance;
		return _instance;
	}
	~Search_HttpServer();

	int start_server(int port);
	int stop_server();

private:
	Search_HttpServer();
	static unsigned int __stdcall http_server_thread(void* param);
	int http_server_thread_aid();
	
	int search_words(std::string quetion, std::string& answer);
	int recv_request(int sock, std::string& question);
	int send_response(int sock, std::string& answer);


	std::string url_decode(std::string source_url);
	int  char_to_dec(char c);

	int          m_thread_handle;
	SOCKET  m_listen_socket;
	bool       m_exit;
};

#endif // !1
