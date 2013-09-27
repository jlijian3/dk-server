
/*
 * Copyright (C) lijian2@ucweb.com
 */

#ifndef __DKINNER_THREAD_POOL_INCLUDE__
#define __DKINNER_THREAD_POOL_INCLUDE__

#include <vector>

#include "dk_core.h"

class DKThreadPool {
public:
  DKThreadPool() {
  }
  
  ~DKThreadPool() {
  }

  bool Init(int threads);

  bool CallInThread(deferred_cb_fn cb, void *arg);

private:
  std::vector<DKWorker>   workers_;
  sem_t                       sem_;
  LockQueue<DeferredCb>       que_;
};

#endif
