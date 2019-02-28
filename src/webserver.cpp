#include <iostream>
#include <unistd.h>

#include "Runnable.h"
#include "Socket.h"
#include "Queue.h"
#include "Server.h"







int main() {
	bool ok = false;
	Server* server = new Server(8069);
	ok = server->Start();
	if (!ok) {
		printf("error starting server\n");
	}
	::usleep(500);

	Socket* sock = new Socket();

	bool yep = sock->connect("192.168.194.247:8069");
	if (!yep) {
		printf("client cant connect\n");
	}
	yep = sock->send(""
		"POST / HTTP/1.0\n"
		"\r\n"
		"farts\r\n");
	if (!yep) {
		printf("client couldn't send\n");
	}
	std::string s = "";
	sock->recv(s);
	printf("--CLIENT RECEIVED--\n%s\n", s.c_str());


	delete sock;


	::usleep(500);

	server->Stop();

	delete server;
}






