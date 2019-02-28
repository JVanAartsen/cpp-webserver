#ifndef def_Server
#define def_Server



#include "Thread.h"
#include "Socket.h"
#include "Queue.h"
#include "Runnable.h"
#include <string.h>
#include <thread>


class RequestWorker {
private:
	Queue<Socket*>* incomingConnections;
	std::thread* m_thread;
public:
	RequestWorker(Queue<Socket*>* rq);
	~RequestWorker();
	static void loop(void*);
};


class Server {
private:
	volatile bool m_running;
	volatile bool m_stoprequested;
	int port;
	const static int num_threads = 5;
	Socket* listener;
	std::thread m_listener_thread;
	RequestWorker *m_workers[num_threads];
	Queue<Socket*>* socketQueue;
	static void start_listener_thread(void*);
	void loop(void);
public:
	Server(int port);
	~Server();
	bool Start(void);
	bool Stop(void);
};


#endif
