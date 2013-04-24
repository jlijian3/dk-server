/**
 * author:lijian2@ucweb.com
 * date:2012-03-08
 */

#ifndef __DONKEY_BASE_CONNECTION_INCLUDE__
#define __DONKEY_BASE_CONNECTION_INCLUDE__

#include "donkey_common.h"
#include "donkey_internal.h"
#include "donkey_util.h"

struct event_base;

class DonkeyServer;

class DonkeyBaseConnection {
public:
  DonkeyBaseConnection();
  virtual ~DonkeyBaseConnection();
  
  bool Init(struct event_base *base,
            int fd,
            const char *host,
            unsigned short port);
   
  void Destruct();

  void Reset();

  bool Connect();
  void ConnectMade();

  bool StartRead();
  bool StartWrite();

  void EnableRead() {
    int event = bufferevent_get_enabled(bufev_);
    if (event & EV_READ)
      return;
    bufferevent_enable(bufev_, EV_READ); 
  }

  void EnableWrite() {
    int event = bufferevent_get_enabled(bufev_);
    if (event & EV_WRITE)
      return;
  	bufferevent_enable(bufev_, EV_WRITE); 
  }

  void DisableRead() {
    int event = bufferevent_get_enabled(bufev_);
    if (event & EV_READ)
      bufferevent_disable(bufev_, EV_READ); 
  }

  void DisableWrite() {
    int event = bufferevent_get_enabled(bufev_);
    if (event & EV_WRITE)
      bufferevent_disable(bufev_, EV_WRITE);  
  }

  void DisableReadWrite() {
    int old = bufferevent_get_enabled(bufev_);
    if (old)
      bufferevent_disable(bufev_, old); 
  }

  bool Send(struct evbuffer *buf) {
    if (kind_ == CON_OUTGOING) {
      if (!IsConnected()) {
        if (!Connect())
          return false;
        else
          return 0 == evbuffer_add_buffer(temp_output_buf_, buf);
      }

      AddOutputBuffer(buf);
      return StartWrite();
    } else {
      AddOutputBuffer(buf);
      return StartWrite();
    }

    return true;
  }

  bool Send(const std::string &buf) {
    return Send(buf.data(), buf.size());
  }

  bool Send(const void *buf, int len) {
    if (kind_ == CON_OUTGOING) {
      if (!IsConnected()) {
        if (!Connect())
          return false;
        else
          return 0 == evbuffer_add(temp_output_buf_, buf, len);
      }

      AddOutputBuffer(buf, len);
      return StartWrite();
    } else {
      AddOutputBuffer(buf, len);
      return StartWrite();
    }

    return true;
  }

  void AddOutputBuffer(struct evbuffer *buf) {
    assert(buf);
    assert(bufev_);
    bufferevent_write_buffer(bufev_, buf); 
  }

  void AddOutputBuffer(const std::string &data) {
    assert(bufev_);
    evbuffer_add(get_output_buffer(), data.data(), data.size()); 
  }

  void AddOutputBuffer(const void *data, int len) {
    assert(bufev_);
    evbuffer_add(get_output_buffer(), data, len); 
  }

  bool IsConnected() {
    switch (state_) {
      case DKCON_DISCONNECTED:
      case DKCON_CONNECTING:
        return false;
        
      default:
        return true; 
    }
  }

  struct evbuffer *get_input_buffer() {
    return bufferevent_get_input(bufev_); 
  }

  struct evbuffer *get_output_buffer() {
    return bufferevent_get_output(bufev_); 
  } 

  int get_fd() {
    return fd_;
  }

  void set_fd(int fd) {
    fd_ = fd; 
  }

  bool keep_alive() {
    return keep_alive_;
  }

  void set_keep_alive(bool keep_alive) {
    keep_alive_ = keep_alive; 
  }

  const std::string &get_host() {
    return host_;
  }

  unsigned short get_port() {
    return port_;
  }

  DonkeyConnectionState get_state() {
    return state_;
  }

  void set_state(DonkeyConnectionState state) {
    state_ = state; 
  }

  struct event_base *get_base() {
    return base_;
  }

  DonkeyConnectionError get_error() {
    return error_; 
  }

  const char *get_error_string() {
    return StrError(error_);
  }

  int get_id() {
    return id_;
  }

  int get_timeout() {
    return timeout_;
  }

  void set_timeout(int timeout) {
    if (!bufev_)
      return;

    timeout_ = timeout;
    if (timeout_ != -1)
      bufferevent_settimeout(bufev_, timeout_, timeout_);
    else
      bufferevent_settimeout(bufev_, DK_READ_TIMEOUT, DK_WRITE_TIMEOUT);
  }
   
  static const char *StrError(DonkeyConnectionError error) {
    switch (error) {
    case DKCON_ERROR_TIMEOUT:
      return "connection timeout";

    case DKCON_ERROR_EOF:
      return "connection close";

    case DKCON_ERROR_ERRNO:
      return strerror(errno);
    
    case DKCON_ERROR_BUFFER:
      return "read or write error";
    
    case DKCON_ERROR_PARSE_ERROR:
      return "parse error";
    
    case DKCON_ERROR_NONE:
    default:
      return "none";
    }
  }

public:
  void Fail(DonkeyConnectionError error);
  void ConnectFail(DonkeyConnectionError error);

protected:
  void AddToFreeConn();

  virtual void ConnectedCallback() {}
  virtual void CloseCallback() {} 
  virtual void ErrorCallback(DonkeyConnectionError error) {}
  virtual void WriteCallback() {}
  virtual void ResetCallback() {}

  virtual enum READ_STATUS ReadCallback() {
    evbuffer_drain(bufferevent_get_input(bufev_), -1); 
    return READ_ALL_DATA; 
  } 
    

private:
  void WriteDone();
  void ReadDone(); 

  void ReadHandler();
  
  static void EventReadCb(struct bufferevent *bufev, void *arg);

  static void EventWriteCb(struct bufferevent *bufev, void *arg);
  
  static void EventErrorCb(struct bufferevent *bufev,
                           short which,
                           void *arg);

  static void EventConnectCb(struct bufferevent *bufev,
                             short which,
                             void *arg);  

public:
   DonkeyServer     *server_; 

protected:
  bool               inited_;
  struct event_base *base_;
  std::string        host_;
  unsigned short     port_;
  int                fd_;
  bool               keep_alive_; 
  int                timeout_;
  int                id_;

  struct bufferevent      *bufev_;
  DonkeyConnectionState    state_;
  DonkeyConnectionKind     kind_;
  DonkeyConnectionError    error_;
  struct evbuffer         *temp_output_buf_;

private:
  static int last_conn_id_;  
};

#endif
