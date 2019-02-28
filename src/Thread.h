#ifndef def_Thread
#define def_Thread

#include "Runnable.h"
#include <pthread.h>

class Thread{
private:
	// volatile is a keyword that tells compiler not to aggressively optimize this memory access, as it could be mutated from like anywhere
	volatile bool m_stoprequested;
	volatile bool m_running;
	pthread_t m_thread;
	Runnable* workBot;
	static void* start_thread(void* obj) {
		((Thread*)obj)->run();
		return NULL;
	}
	void run();
public:
	Thread(Runnable* bot);
	~Thread();
	void start();
	void stop();
};

#endif
