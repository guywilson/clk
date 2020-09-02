#include <stdlib.h>
#include <string.h>

#include "posixthread.h"
#include "webadmin.h"
#include "logger.h"
#include "clk_error.h"
#include "threads.h"

void ThreadMgr::startThreads()
{
	Logger & log = Logger::getInstance();
	
	this->webListenThread = new WebListenThread();
	
	if (this->webListenThread->start()) {
		log.logStatus("WebListenThread started successfully");
	}
	else {
		log.logError("Faied to start WebListenThread");
	}
}

void * WebListenThread::run()
{
	WebAdmin & web = WebAdmin::getInstance();
	Logger & log = Logger::getInstance();

	web.listen();

	log.logError("web.listen returned...");

	return NULL;
}
