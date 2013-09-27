
/*
 * Copyright (C) lijian2@ucweb.com
 */

#include "dk_common.h"
#include "dk_base_thread.h"

void DKBaseThread::Create() {
  pthread_attr_t  attr;
  int             ret;

  pthread_attr_init(&attr);

  if ((ret = pthread_create(&thread_id_, &attr,
                            PThreadRoutine, (void *)this)) != 0) {
    fprintf(stderr, "Can't create thread: %s\n", strerror(ret));
    exit(1);
  }
}

void *DKBaseThread::PThreadRoutine(void *arg) {
  assert(arg);
  int rv;

  DKBaseThread *me = (DKBaseThread *)arg;
  rv = me->ThreadRoutine();
  pthread_exit((void *)&rv);
}


