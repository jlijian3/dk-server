
/*
 * Copyright (C) lijian2@ucweb.com
 */

#include "donkey_common.h"
#include "donkey_base_thread.h"

void DonkeyBaseThread::Create() {
  pthread_attr_t  attr;
  int             ret;

  pthread_attr_init(&attr);

  if ((ret = pthread_create(&thread_id_, &attr,
                            PThreadRoutine, (void *)this)) != 0) {
    fprintf(stderr, "Can't create thread: %s\n", strerror(ret));
    exit(1);
  }
}

void *DonkeyBaseThread::PThreadRoutine(void *arg) {
  assert(arg);
  int rv;

  DonkeyBaseThread *me = (DonkeyBaseThread *)arg;
  rv = me->ThreadRoutine();
  pthread_exit((void *)&rv);
}


