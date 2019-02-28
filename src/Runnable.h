#ifndef def_Runnable
#define def_Runnable
#include <stdio.h>
// this is just an interface that will be implemented by other things
class Runnable {
public:
	virtual ~Runnable(){
		::printf("fuck this\n");
	}; // mmmm idk
	bool ceaseThread; // these variables are how we'll communicate to the thread wrapper implementation when we want to kill the thread
	bool killThread;
	virtual void run(void) = 0; // more pure virtual
	virtual void error(void) = 0;
};


#endif
