

#include "Server.h"
#include <string>
#include <thread>



RequestWorker::RequestWorker(Queue<Socket*>* rq) {
	incomingConnections = rq;
	this->m_thread = new std::thread(RequestWorker::loop,(void*) this);
};
RequestWorker::~RequestWorker() {
	printf("RequestWorker cleaning up..\n");
	delete this->m_thread;
}

void RequestWorker::loop(void* obj) {
	RequestWorker* me = (RequestWorker*) obj;

	while (true) {
		// TODO we might not want to block threads that are in a threadpool, it makes them
		// hard to join. Ik a webserver doesn't usually need to shut down gracefully
		// but it would be nice *shrug*
		Socket* client_socket = me->incomingConnections->Dequeue(); // blocking
		//client_socket->set_non_blocking(true);
		std::string reqString;

		// need to allocate a relateively large buffer if we wanna read this all in
		// or keep raising the size of
		printf("\nRECEIVING\n");
		while(client_socket->recv(reqString) == SOCK_MAXRECV) {
			printf("%s\n", reqString.c_str());
		}

		std::string respString = std::string(""
		"HTTP/1.1 200 OK\n"
		"Connection: Close\n"
		"Content-Type: text/html; charset=iso-8859-1\n"
		"\r\n"
		"<html><body><h1>JAVES FTW</h1></body></html>\n"
		"\r\n"
		);

		bool ok = client_socket->send(respString);
		if (!ok) {
			printf("well shit\n");
		}
		delete client_socket;
	}
};



Server::Server(int port_arg) {
	printf("creating server...\n");
	this->m_running = this->m_stoprequested = false;
	this->port = port_arg;
	this->socketQueue = new Queue<Socket*>();
	this->listener = new Socket();
	for (int i = 0; i < this->num_threads; i++) {

	}
}

Server::~Server() {
	::printf("destroying server...\n");
	if (this->m_running) {
		this->Stop();
	}
	delete this->socketQueue;
	delete this->listener;

}


void Server::start_listener_thread(void* obj) {
	((Server*)obj)->loop();

}

void Server::loop(void) {
	printf("server looping \n");
	for (int i = 0; i < this->num_threads; i++) {
		this->m_workers[i] = new RequestWorker(this->socketQueue);
	}

	while (!this->m_stoprequested) {
		Socket* client_socket = new Socket();
		listener->accept(*client_socket); // blocking
		socketQueue->Enqueue(client_socket); // blocking
	}
}

bool Server::Start(void) {
	if(!listener->bind(this->port)) {
		return false;
	}
	if(!listener->listen()) {
		printf("bad listener\n");
	}

	this->m_running = true;
	this->m_listener_thread = std::thread(Server::start_listener_thread, (void*) this);

	printf("Listening on port %s\n", std::to_string(this->port));
	return true;

}

bool Server::Stop(void) {
	this->m_stoprequested = true;
	for (int i = 0; i < this->num_threads; i++) {
		delete (this->m_workers[i]);
	}
	this->m_running = false;
	this->m_stoprequested = false;

	return true;
}
