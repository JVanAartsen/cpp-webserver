#ifndef def_Socket
#define def_Socket


#include <sys/socket.h>
#include <sys/types.h>
#include <string>
#include <netinet/in.h>// https://pubs.opengroup.org/onlinepubs/7908799/xns/netinetin.h.html
#include <netdb.h>

const int MAXCONNECTIONS = 5;
const int SOCK_MAXRECV = 1000;


class Socket{
private:
	int m_descriptor;

	addrinfo* m_addr_list; // linked list of potential addresses matched by "hostname:port"
	bool set_addr(std::string);
public:
	sockaddr m_addr;
	Socket();
	virtual ~Socket(); // ~ClassName indicates the destructor which is called when instance goes out of scope or `delete`ed
	// server
	bool create();
	bool bind(const int port);
	bool listen(void) const; // this trailing const means this method cannot mutate member variables, recommended to avoid accidental mutates i guess
	bool accept(Socket&) const; // trailing ampersand is a reference type, a little different from a pointer
	// client
	bool connect(const std::string);
	std::string getIP() const;
	unsigned short getPort() const;
	// data transmission
	bool send(const std::string) const;
	int recv(std::string&) const;

	void set_non_blocking(const bool);
	bool is_valid() const;
};


#endif
