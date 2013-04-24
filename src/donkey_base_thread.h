
/*
 * Copyright (C) lijian2@ucweb.com
 */

#ifndef __DONKEY_BASE_THREAD_INCLUDE__
#define __DONKEY_BASE_THREAD_INCLUDE__

#include <pthread.h>

class DonkeyBaseThread {

public:
  DonkeyBaseThread() : thread_id_(0) {
  }

  virtual ~DonkeyBaseThread() {
  }

  void Create();
  
  int Wait() {
    return pthread_join(thread_id_, NULL);
  }

  virtual bool Stop() {
    return true; 
  };
  
  pthread_t get_thread_id() {
    return thread_id_;
  }

protected:
  virtual int ThreadRoutine() = 0;
  static void *PThreadRoutine(void *arg);

private:
  pthread_t thread_id_;
};

#endif
