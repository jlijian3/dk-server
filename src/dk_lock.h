#ifndef __DK_LOCK__
#define __DK_LOCK__
#include <pthread.h>
#include <exception>

class Mutex
{
public:
	Mutex() {
    pthread_mutex_init(&mtx_, NULL);
	};

	~Mutex() {
		pthread_mutex_destroy(&mtx_); 
	};

  void lock() {
    pthread_mutex_lock(&mtx_);
  }

  void unlock() {
    pthread_mutex_unlock(&mtx_);
  }

private:
  pthread_mutex_t mtx_; 
};

class Lock {
public:
  Lock(Mutex &mtx) {
    pmtx_ = &mtx;
    mtx.lock();
  }

  ~Lock() {
    if (pmtx_)
      pmtx_->unlock();
  }
private:
  Mutex *pmtx_;
};

#endif

