#include "posixthread.h"

#ifndef __INCL_THREAD
#define __INCL_THREAD

class ListenThread : public PosixThread
{
    public:
        virtual void *      run();
};

#endif
