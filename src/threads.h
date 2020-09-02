#include "posixthread.h"

#ifndef _INCL_THREADS
#define _INCL_THREADS

class WebListenThread : public PosixThread
{
public:
	WebListenThread() : PosixThread(true) {}
	
	virtual void *	run();
};


class ThreadMgr
{
public:
	static ThreadMgr & getInstance() {
		static ThreadMgr instance;
		return instance;
	}

private:
	ThreadMgr() {}
	
	WebListenThread *		webListenThread = NULL;

public:
	void startThreads();
	void stopThreads();	
};

#endif
