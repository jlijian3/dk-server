
/*
 * Copyright (C) jlijian3@gmail.com
 */

#ifndef __DONKEY_WORKER__INCLUDE__
#define __DONKEY_WORKER__INCLUDE__

#include "dk_queue.h"
#include "dk_base_thread.h"
#include "dk_internal.h"

class DKWorker : public DKBaseThread {
public:
  DKWorker()
      : stop_(false),
        extern_event_sem_(NULL),
        extern_pending_cbs_(NULL) {
  }

  bool Init() {
    return 0 == sem_init(&event_sem_, 0, 0); 
  }

  virtual ~DKWorker() {
  }

  virtual bool Stop() {
    stop_ = true;
    return true;
  }

  size_t PendingQueSize() {
    return pending_cbs_.size();
  }

  virtual int ThreadRoutine();

  bool CallInThread(deferred_cb_fn cb, void *arg) {
    LockQueue<DeferredCb> *aque =
       extern_pending_cbs_ ? extern_pending_cbs_ : &pending_cbs_;
    aque->push(DeferredCb(cb, arg));
    sem_t *asem = extern_event_sem_ ? extern_event_sem_ : &event_sem_;
    return 0 == sem_post(asem);
  }

  void set_extern_sem_queue(sem_t *asem, LockQueue<DeferredCb> *aque) {
    extern_event_sem_ = asem;
    extern_pending_cbs_ = aque; 
  }

protected:
  bool                   stop_;
  sem_t                  event_sem_;
  LockQueue<DeferredCb>  pending_cbs_;

  sem_t                 *extern_event_sem_;
  LockQueue<DeferredCb> *extern_pending_cbs_;
};

#endif
