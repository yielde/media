#include "thread.h"

std::map<pthread_t, CThread *> CThread::m_mapThread;