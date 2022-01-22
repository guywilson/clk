#include "thread.h"
#include "webadmin.h"

void * ListenThread::run()
{
    WebAdmin & web = WebAdmin::getInstance();

    web.initListener();
    web.listen();

    return nullptr;
}
