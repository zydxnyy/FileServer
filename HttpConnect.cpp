
#include "HttpConnect.h"



#ifdef WIN32

#pragma comment(lib,"ws2_32.lib")

#endif



HttpConnect::HttpConnect(int port)
{
	this->port = port;
	reply = new char[1024 * 1024];
#ifdef WIN32

	//�˴�һ��Ҫ��ʼ��һ�£�����gethostbyname����һֱΪ��

	WSADATA wsa = { 0 };

	WSAStartup(MAKEWORD(2, 2), &wsa);

#endif

}



HttpConnect::~HttpConnect()

{



}

void HttpConnect::socketHttp(string host, string request)

{
	memset(reply, 0, 1024 * 1024);

	int sockfd;

	struct sockaddr_in address;

	struct hostent *server;



	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	address.sin_family = AF_INET;

	address.sin_port = htons(port);

	server = gethostbyname(host.c_str());

	memcpy((char *)&address.sin_addr.s_addr, (char*)server->h_addr, server->h_length);



	if (-1 == connect(sockfd, (struct sockaddr *)&address, sizeof(address))) {

		DBG << "connection error!" << std::endl;

		return;

	}



	//DBG << request << std::endl;

#ifdef WIN32

	send(sockfd, request.c_str(), request.size(), 0);

#else

	write(sockfd, request.c_str(), request.size());

#endif


	int offset = 0;

	int rc;



#ifdef WIN32

	while (rc = recv(sockfd, reply + offset, 1024, 0))

#else

	while (rc = read(sockfd, reply + offset, 1024))

#endif

	{

		offset += rc;

	}



#ifdef WIN32

	closesocket(sockfd);

#else

	close(sockfd);

#endif

	reply[offset] = 0;

	//DBG << reply << std::endl;
}



string HttpConnect::postData(string host, string path, string post_content)

{
	//POST����ʽ
	std::stringstream stream;

	stream << "POST " << path;

	stream << " HTTP/1.0\r\n";

	stream << "Host: " << host << "\r\n";

	stream << "User-Agent: Mozilla/5.0 (Windows; U; Windows NT 5.1; zh-CN; rv:1.9.2.3) Gecko/20100401 Firefox/3.6.3\r\n";

	stream << "Content-Type:application/x-www-form-urlencoded\r\n";

	stream << "Content-Length:" << post_content.length() << "\r\n";

	stream << "Connection:close\r\n\r\n";

	stream << post_content.c_str();

	socketHttp(host, stream.str());

	return reply;
}


string HttpConnect::getData(string host, string path, string get_content)
{
	//GET����ʽ
	std::stringstream stream;

	stream << "GET " << path << "?" << get_content;

	stream << " HTTP/1.0\r\n";

	stream << "Host: " << host << "\r\n";

	stream << "User-Agent: Mozilla/5.0 (Windows; U; Windows NT 5.1; zh-CN; rv:1.9.2.3) Gecko/20100401 Firefox/3.6.3\r\n";

	stream << "Connection:close\r\n\r\n";
	socketHttp(host, stream.str());
	return reply;

}