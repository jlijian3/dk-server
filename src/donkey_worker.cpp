
/*
 * Copyright (C) lijian2@ucweb.com
 */

#include "donkey_common.h"
#include "donkey_worker.h"

int DonkeyWorker::ThreadRoutine() {
	int ret = 0;

  sem_t *asem = extern_event_sem_ ? extern_event_sem_ : &event_sem_;
  LockQueue<DeferredCb> *aque =
      extern_pending_cbs_ ? extern_pending_cbs_ : &pending_cbs_;

	for ( ; !stop_ ; ) {
		/* no events and pending cb */
		if (pending_cbs_.empty()) {
			struct timespec ts;
			if (clock_gettime(CLOCK_REALTIME, &ts) == 0) {
				ts.tv_sec += 1;
				if (sem_timedwait(asem, &ts) == -1) {
					if (errno == EINTR) {
						continue;
					} else if (errno == ETIMEDOUT) {
					} else {
						perror("sem_timedwait");
						break;
					}
				}
			}
		}

		for ( ; !aque->empty(); ) {
			try {
				DeferredCb deferred_cb = aque->pop();  
				deferred_cb.Call(this);
			} catch (const std::exception e) {
				break;
			}
		}
	}

	return ret;
}
