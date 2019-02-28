#include "Thread.h"
#include <iostream>

// 97826
// again, before we jump in lets understand the system level interface that we're writing a wrapper for
// pthread_create (*thread, *attr, *start_routine, *arg)
// 	this will create a thread at *thread, attrs are like thread create options. kicks off thread exec by start_routine(args)
// pthread_cancel (thread) -- exactly what is sounds like, cancel execution of the thread, stop it in its tracks
// pthread_join (thread, **retval) -- waits for it to finish execution, puts pointer to return val in **retval
// pthread_mutex_...

// TODO notes on the relationship / model of threads and workers
void Thread::run() {
	while(!m_stoprequested) {
		if(workBot->ceaseThread) {
			stop();
			return;
		}
		try {
			workBot->run();
		} catch(...) {
			std::cout << "An error occurred in run() \n";
			workBot->error();
		}
	}
}

Thread::Thread(Runnable* bot) : m_stoprequested(false), m_running(false) {
	::printf("creating thread...\n");
	workBot = bot;
	workBot->ceaseThread = workBot->killThread = false; // multiple assignment c++ style
}

Thread::~Thread() {
	this->stop();
	printf("threxiting\n");
	pthread_detach(this->m_thread);
}

void Thread::start() { 
	if(m_running) {
		return;
	}
	m_running = true;
	// TODO ok, theres a lot to unpack here
	pthread_create(&m_thread, NULL, Thread::start_thread, this);

}

void Thread::stop() {
	if(!m_running) {
		return;
	}
	m_running = false;
	m_stoprequested = true;
	pthread_cancel(m_thread);
	pthread_join(m_thread, NULL);

}
