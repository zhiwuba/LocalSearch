#ifndef  __SEARCH_HTTP_SERVER_H__
#define __SEARCH_HTTP_SERVER_H__

#include <string>
#include "search_porting.h"

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
#ifdef WIN32
	static unsigned int __stdcall http_server_thread(void* param);
#else
	static void* http_server_thread(void* param);
#endif
	int http_server_thread_aid();
	
	int search_words(std::string quetion, std::string& answer);
	int recv_request(int sock, std::string& question);
	int send_response(int sock, std::string& answer);


	std::string url_decode(std::string source_url);
	int  char_to_dec(char c);

	handle_thread m_thread_handle;
	SOCKET  m_listen_socket;
	bool       m_exit;
};

#endif // !1
