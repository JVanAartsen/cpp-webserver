#include "Socket.h"
#include <string>
#include <iostream>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>// https://pubs.opengroup.org/onlinepubs/7908799/xns/netinetin.h.html
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>


// kind of nice to know the system interface that we're gonna wrap here
// https://www.geeksforgeeks.org/socket-programming-cc/
// and this article for when you find yourself in a "wtf why" scenario and need the deets
// https://beej.us/guide/bgnet/html/multi/index.html


Socket::Socket() {
	this->m_descriptor = socket(AF_INET, SOCK_STREAM, 0); // socket(domain: ipv4, type: TCP, protocol: 0 is IP)
	if(!this->is_valid()) {
		throw "ahh shit";
	}
	memset(&this->m_addr, 0, sizeof this->m_addr);
	memset(&this->m_addr_list, 0, sizeof this->m_addr_list);
	int on = 1;
	// alot to unpack in setsockopt or set socket options,
	setsockopt(this->m_descriptor, 
		SOL_SOCKET, // level at which options apply, SOL_SOCKET is "this socket only"
		SO_REUSEADDR, // allows local reuse of address
		(const char*)&on, // cast to a pointer to constant, the value of which tells whether the options are on or off
		sizeof(on)); // and it needs to know the length of it too since opts can be structs

	//this->set_addr(hostname);
}

Socket::~Socket() {
	if(is_valid()) {
		::close(this->m_descriptor);
	}
	printf("destroying socket\n");
	freeaddrinfo(this->m_addr_list); // lil utility function to free mem. im fucking something up here, prob because of the accept() func but *shrug*
}

bool Socket::create() {
	this->m_descriptor = socket(AF_INET, SOCK_STREAM, 0); // socket(domain: ipv4, type: TCP, protocol: 0 is IP)
	if(!is_valid()) {
		return false;
	}
	int on = 1;
	// alot to unpack in setsockopt or set socket options,
	int rc = setsockopt(this->m_descriptor, 
		SOL_SOCKET, // level at which options apply, SOL_SOCKET is "this socket only"
		SO_REUSEADDR, // allows local reuse of address
		(const char*)&on, // cast to a pointer to constant, the value of which tells whether the options are on or off
		sizeof(on)); // and it needs to know the length of it too since opts can be structs
	return rc; // return code of our sockopts, 0 success
}
//m_addr_list	0x00000000006191a0

bool Socket::bind(const int port) {
	if(!is_valid()) {
		return false;
	}

	std::string port_str = std::string(":") + std::to_string(port);
	if (!this->set_addr(port_str)) {
		return false;
	}

	for (addrinfo* addr_node = this->m_addr_list; addr_node != NULL; addr_node = addr_node->ai_next) {
		int rc = ::bind(
			this->m_descriptor, // our descriptor for the socket
			addr_node->ai_addr,
			addr_node->ai_addrlen);

		if (rc == 0) { // success, we binded
			this->m_addr = *(addr_node->ai_addr);
			//freeaddrinfo(this->m_addr_list);
			return true;
		}
	}

	return false;
}

bool Socket::listen(void) const {
	if(!is_valid()) {
		return false;
	}
	// under the hood, connections specifies the length of the queue for the pending connections to this->descriptor
	// if queue is full the client gets ECONNREFUSED
	int rc = ::listen(this->m_descriptor, MAXCONNECTIONS);
	return rc != -1;
}

// sockets are a client-server model, so a client socket connects to listening server socket
// we need to tell the OS that our server socket is ready to handle the pending socket and open it to begin reading from its buffer
// notice we accept OUR implementation of Socket and not the sys/socket.
// also notice we give the pointer to where in mem the sockets address is so accept() can fill that out for us
// and we have a legit Socket() instance =)
bool Socket::accept(Socket& new_socket) const {
	sockaddr_storage their_addr;
	socklen_t addr_size =  sizeof new_socket.m_addr;

	new_socket.m_descriptor = ::accept(
		this->m_descriptor,
		(sockaddr*) &their_addr,
		(socklen_t*) &addr_size
	);

	if (new_socket.m_descriptor < 1) {
		printf("oopsie accepting: %s\n", strerror(errno));
		return false;
	}

	new_socket.m_addr = *((sockaddr*) &their_addr); // yikes what

	return true;
}

bool Socket::send(const std::string s) const {
	if(!is_valid()) {
		return false;
	}
	int bytes_sent = ::send(this->m_descriptor, s.c_str(), s.size(), 0);
	if (bytes_sent == -1) {
		printf("oopsie sending: %s\n", strerror(errno));
		return false;
	}
	return true;
}

// s is like a return address parameter. the caller gives the location of where to write
// the received message into. makes it easier to recv since caller can use the string class instead of dealing with a buffer
// that and now we can set some socket-specific behavior for buffering in that data
int Socket::recv(std::string& s) const{
	char buf[SOCK_MAXRECV+1]; // array of char types, +1 to allow for null termination
	s = ""; // 
	memset(buf, 0, SOCK_MAXRECV+1); // zero our mem here, we cant use sizeof here because sizeof goes until null terminator,
	int size = ::recv(this->m_descriptor, buf, SOCK_MAXRECV, 0); // returns 

	if (size == -1) {
		printf("\noopsie recving: %s\n", strerror(errno));
		return 0;
	}
	else if (size == 0) {
		return 0;
	} else {
		s = buf;
		return size;
	}
}

bool Socket::connect(const std::string hostname) {
	if(!is_valid()) {
		return false;
	}
	if(!this->set_addr(hostname)) {
		return false;
	}
	for (addrinfo* addr_node = this->m_addr_list; addr_node != NULL; addr_node = addr_node->ai_next) {
		int rc = ::connect(this->m_descriptor, addr_node->ai_addr, addr_node->ai_addrlen);

		if (rc != -1) {
			this->m_addr = *(addr_node->ai_addr);
			//freeaddrinfo(this->m_addr_list);
			return true;
		} 
	}
	return false;
}


bool Socket::set_addr(std::string hostname) {
	size_t colonPos = hostname.find(":");
	std::string host = hostname.substr(0, colonPos);
	std::string port = hostname.substr(colonPos+1);

	struct addrinfo hints;
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET; // ivp4
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	// host="" or NULL will give us localhost
	int rc = ::getaddrinfo(NULL, port.c_str(), &hints, &this->m_addr_list);
	if ( rc != 0 ) {
		::printf("getaddrinfo error: %s\n", gai_strerror(rc));
		return false;
	} else {
		return true;
	}
}

// you could probably merge getIP and getPort into one function, but i don't want to deal with retunring multiple values
std::string Socket::getIP() const{
	struct sockaddr_in peer_addr;
	socklen_t len = sizeof peer_addr;
	// returns the address of the peer that the socket has connected to
	getpeername(this->m_descriptor, (struct sockaddr*)&peer_addr, &len); // why pass reference to len?
	return inet_ntoa(peer_addr.sin_addr);
}

unsigned short Socket::getPort() const{
	struct sockaddr_in peer_addr;
	socklen_t len = sizeof peer_addr;
	getpeername(this->m_descriptor, (struct sockaddr*)&peer_addr, &len);
	return peer_addr.sin_port;
}

// we 
void Socket::set_non_blocking(const bool B) {
	int opts;
	opts = fcntl(this->m_descriptor, F_GETFL);
	if (opts < 0) {
		return;
	}
	if(B) {
		opts = (opts | O_NONBLOCK);
	} else {
		opts = (opts & ~O_NONBLOCK);
	}
	fcntl(this->m_descriptor, F_SETFL, opts);
}

bool Socket::is_valid() const{
	return this->m_descriptor != -1;
}
