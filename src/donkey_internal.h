/**
 * author:lijian2@ucweb.com
 * date:2012-03-23
 */

#ifndef __DONKEY_INTERVAL_INCLUDE__
#define __DONKEY_INTERVAL_INCLUDE__

class DonkeyBaseThread;

enum READ_STATUS {
  READ_BAD_CLIENT,
  READ_NEED_MORE_DATA,
  READ_ALL_DATA,
  READ_MEMORY_ERROR,
  READ_INNER_ERROR
};

enum DonkeyConnectionKind {
  CON_INCOMING,
  CON_OUTGOING
};

#define DK_CONNECT_TIMEOUT 45
#define DK_READ_TIMEOUT    50
#define DK_WRITE_TIMEOUT   50

enum DonkeyConnectionState {
  DKCON_DISCONNECTED,
  DKCON_CONNECTING,
  DKCON_IDLE,
  DKCON_READING,
  DKCON_WRITING
};

enum DonkeyConnectionError {
  DKCON_ERROR_NONE, 
  DKCON_ERROR_TIMEOUT,
  DKCON_ERROR_EOF,
  DKCON_ERROR_ERRNO, 
  DKCON_ERROR_BUFFER,
  DKCON_ERROR_PARSE_ERROR
};

typedef void (*deferred_cb_fn)(DonkeyBaseThread *thread, void *arg);

class DeferredCb {
public:
  DeferredCb(deferred_cb_fn cb, void *arg)
      : cb_(cb), arg_(arg) {
  }

  void Call(DonkeyBaseThread *thread) {
    if (cb_)
      cb_(thread, arg_);
  }

private:
  deferred_cb_fn cb_;
  void *arg_;
};


#endif
