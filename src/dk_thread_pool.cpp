
/*
 * Copyright (C) lijian2@ucweb.com
 */

#include "dk_thread_pool.h"

bool DKThreadPool::Init(int threads) {
  if (threads <= 0)
    return false;

  if (sem_init(&sem_, 0, 0) != 0){
    return false;
  }

  workers_.resize(threads);
  for (size_t i = 0; i < workers_.size(); i++) {
    if (!workers_[i].Init())
      return false;

    workers_[i].set_extern_sem_queue(&sem_, &que_);
    workers_[i].Create();
  }

  return true;
}

bool DKThreadPool::CallInThread(deferred_cb_fn cb, void *arg) {
  que_.push(DeferredCb(cb, arg));
  return 0 == sem_post(&sem_);
}
