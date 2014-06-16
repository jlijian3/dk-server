/**
 * author:jlijian3@gmail.com
 * date:2012-03-08
 */

#include "dk_core.h"
#include "dk_util.h"

using namespace std;

int DKBaseConnection::last_conn_id_;

const char *DKCON_STATE_NAMES[] = {
    "DKCON_DISCONNECTED",
    "DKCON_CONNECTING",
    "DKCON_IDLE",
    "DKCON_READING",
    "DKCON_WRITING"
};

DKBaseConnection::DKBaseConnection()
    : inited_(false), 
      base_(NULL),
      port_(0),
      fd_(-1),
      keep_alive_(false),
      timeout_(-1), 
      id_(-1),
      bufev_(NULL),
      state_(DKCON_DISCONNECTED),
      kind_(CON_INCOMING),
      error_(DKCON_ERROR_NONE),
      temp_output_buf_(NULL),
      bind_port_(0),
      incoming_conn_free_cb_(NULL) {
}

DKBaseConnection::~DKBaseConnection() {
  Destruct();
}

void DKBaseConnection::Destruct() {
  Reset();
  if (bufev_)
    bufferevent_free(bufev_);
  if (temp_output_buf_)
    evbuffer_free(temp_output_buf_);
}

bool DKBaseConnection::Init(struct event_base *base,
                                int fd,
                                const char *host,
                                unsigned short port) {
  if (inited_)
    return true;

  if (!base)
    return false;

  if (fd == -1)
    kind_ = CON_OUTGOING;
  else
    kind_ = CON_INCOMING;

  fd_ = fd;
  base_ = base;
  host_ = host;
  port_ = port;

  if (bufev_) {
    bufferevent_setfd(bufev_, fd_);
    bufferevent_setcb(bufev_,
                      EventReadCb,
                      EventWriteCb,
                      EventErrorCb,
                      this);
  } else {
    bufev_ = bufferevent_new(fd_,
                             EventReadCb,
                             EventWriteCb,
                             EventErrorCb,
                             (void *)this);
  }

  if (!bufev_)
    return false;

  if (!temp_output_buf_) {
    temp_output_buf_ = evbuffer_new();
    if (!temp_output_buf_)
      return false;
  }

  bufferevent_base_set(base_, bufev_);

  id_ = ++last_conn_id_;
  if (id_ == -1)
    id_ = ++last_conn_id_;
 
  inited_ = true;
  keep_alive_ = false;

  return true;
}

bool DKBaseConnection::StartRead() {
  assert(bufev_);

    /* Set up an event to read data */
	DisableWrite();
	EnableRead();
	state_ = DKCON_READING;
	/* Reset the bufferevent callbacks */
	bufferevent_setcb(bufev_,
	    EventReadCb,
      EventWriteCb,
      EventErrorCb,
	    this);  
 
  evbuffer *buf = bufferevent_get_input(bufev_);
  if (evbuffer_get_length(buf) > 0) {
    ReadHandler();
  }

  return true;
}

bool DKBaseConnection::StartWrite() {
  if (!IsConnected()) {
    DK_DEBUG("[warn] %s: not connected\n", __func__);
    return false;
  }

  assert(bufev_);

  if (evbuffer_get_length(get_output_buffer()) == 0)
    return true;

  state_ = DKCON_WRITING; 

  int res = TryWrite();
  if (res < 0) {
    Fail(DKCON_ERROR_ERRNO);
    return false;
  } else if (res == 0) {
    Fail(DKCON_ERROR_EOF);
    return false;
  }

  if (evbuffer_get_length(get_output_buffer()) == 0) {
    WriteDone();
    return true;
  }
  EnableWrite();

	/* Disable the read callback: we don't actually care about data;
	 * we only care about close detection.  (We don't disable reading,
	 * since we *do* want to learn about any close events.) */
  /*
	bufferevent_setcb(bufev_,
	    NULL,
	    EventWriteCb,
	    EventErrorCb,
	    this);
  */
  
  return true;
}

static const int atmost = 16348;
int DKBaseConnection::TryWrite() {
  //return 1;
  evbuffer_unfreeze(get_output_buffer(), 1);
  int res = evbuffer_write(get_output_buffer(), fd_);
  evbuffer_freeze(get_output_buffer(), 1);

  if (res == -1) {
    int err = evutil_socket_geterror(fd_);
    if (err == EINTR || err == EAGAIN)
      return 1;
    else
      return -1;
  } else if (res == 0) {
    return 0;
  }
  return 2;
}

void DKBaseConnection::ReadHandler() {
  if (!bufev_)
    return;

  enum READ_STATUS read_status;
  bool stop = false;
  int nreqs = 200;

  switch (state_) {
  case DKCON_READING:
  case DKCON_IDLE:
    {
      if (state_ == DKCON_IDLE)
        state_ = DKCON_READING;

      while (!stop && nreqs-- > 0) {
        read_status = OnRead();

        switch (read_status) {
        case READ_ALL_DATA:
          ReadDone();
          if (evbuffer_get_length(get_input_buffer()) == 0)
            stop = true;
          break;

        case READ_INNER_ERROR:
        case READ_MEMORY_ERROR:
        case READ_BAD_CLIENT:
          Fail(DKCON_ERROR_PARSE_ERROR);
          stop = true;
          break;

        case READ_NEED_MORE_DATA:
          stop = true;
          break;

        default:
          stop = true;
          break;
        }
      } /* while */
    }
    break;
 
  default:
    DK_DEBUG("[warn] %s state %d is uncorrect\n", __func__, state_);
    break;
  }
}

void DKBaseConnection::ReadDone() {
  if (kind_ == CON_OUTGOING) {
    set_state(DKCON_IDLE);

    if (!keep_alive()) {
      Reset();
      return;
    }

    if (!IsConnected())
      Connect();
    else {
      if (evbuffer_get_length(get_output_buffer()) > 0)
        StartWrite();
    }
    return;
  }
}

void DKBaseConnection::WriteDone() {
  OnWrite();

  if (kind_ == CON_INCOMING) {
    if (!keep_alive()) {
      Reset(); 
      FreeIncomingConn();
      return;
    }
    
    StartRead();
    return;
  }

  if (kind_ == CON_OUTGOING) {
    StartRead();
  }
}

void DKBaseConnection::EventReadCb(struct bufferevent *bufev,
                                   void *arg) {
  DKBaseConnection *conn = (DKBaseConnection *)arg;
  assert(conn);

  conn->ReadHandler();
}

void DKBaseConnection::EventWriteCb(struct bufferevent *bufev,
                                    void *arg) {
  DKBaseConnection *conn = (DKBaseConnection *)arg; 
  assert(conn);

  conn->WriteDone();
}

void DKBaseConnection::EventErrorCb(struct bufferevent *bufev,
                                    short what, 
                                    void *arg) {
  DKBaseConnection *conn = (DKBaseConnection *)arg; 
  assert(conn); 

  switch (conn->get_state()) {
  case DKCON_CONNECTING:
    if (what & BEV_EVENT_TIMEOUT) {
      DK_DEBUG("[error] %s: conecting timeout for %s:%d on %d\n",
          __func__, conn->get_host().c_str(), conn->get_port(),
          conn->get_fd());
      conn->ConnectFail(DKCON_ERROR_TIMEOUT); 
      return;
    }
    break;

  default:
    break;
  }

  if (what & (BEV_EVENT_TIMEOUT|BEV_EVENT_EOF|BEV_EVENT_ERROR)) {
    DKConnectionError error;

    if (what & BEV_EVENT_TIMEOUT) {
      error = DKCON_ERROR_TIMEOUT;
    }
    if (what & BEV_EVENT_EOF) {
      error = DKCON_ERROR_EOF; 
    }
    if (what & BEV_EVENT_ERROR) {
      error = DKCON_ERROR_ERRNO;
    }
    
    conn->Fail(error);
  } else {
    conn->Fail(DKCON_ERROR_ERRNO);
  }
}

void DKBaseConnection::EventConnectCb(struct bufferevent *bufev,
                                      short what,
                                      void *arg) {
  DKBaseConnection *conn = (DKBaseConnection *)arg;
  int error;
	ev_socklen_t errsz = sizeof(error);

  int fd = bufferevent_getfd(bufev);
  conn->set_fd(fd);

  if (!(what & BEV_EVENT_CONNECTED)) {
    EventErrorCb(bufev, what, arg);
    return;
  }

  /* Check if the connection completed */
	if (getsockopt(fd, SOL_SOCKET, SO_ERROR, (void*)&error,
		      &errsz) == -1) {
     
    goto cleanup; 
  }

  if (error) {
    DK_DEBUG("[error] %s: %s %s:%d on %d\n", __func__, strerror(errno),
        conn->get_host().c_str(), conn->get_port(), conn->get_fd());
    goto cleanup;
  }

  conn->ConnectMade();
  return;

cleanup:
  conn->ConnectFail(DKCON_ERROR_ERRNO);
}

void DKBaseConnection::Fail(DKConnectionError error) {
  error_ = error;

  /*
  if (keep_alive() &&
      error == DKCON_ERROR_TIMEOUT &&
      state_ != DKCON_CONNECTING) {
    OnError(error);
    if (state_ == DKCON_WRITING)
      EnableWrite();
    else
      EnableRead();

    error_ = DKCON_ERROR_NONE;
    return;
  }
  */
  
  DK_DEBUG("[error] %s: %s for %s:%d on %d %s\n", __func__, StrError(error),
      get_host().c_str(), get_port(), get_fd(),
      DKCON_STATE_NAMES[get_state()]);

  DisableReadWrite();
  OnError(error);
  Reset();

  if (kind_ == CON_INCOMING) {
    FreeIncomingConn();
    return;
  }
}

void DKBaseConnection::ConnectFail(DKConnectionError error) {
  error_ = error;
  OnError(error);
  Reset();
}

void DKBaseConnection::Reset() {
  if (!bufev_)
    return;

  DisableReadWrite();

  if (fd_ == -1)
    fd_ = bufferevent_getfd(bufev_);

  if (fd_ != -1) {
    //if (IsConnected())
      OnClose();

    shutdown(fd_, SHUT_WR);
    close(fd_);
    fd_ = -1;
    bufferevent_setfd(bufev_, -1);
  }

  struct evbuffer *tmp = get_input_buffer();
	evbuffer_drain(tmp, evbuffer_get_length(tmp));
	tmp = get_output_buffer();
  evbuffer_unfreeze(tmp, 1);	
  evbuffer_drain(tmp, evbuffer_get_length(tmp));
  evbuffer_freeze(tmp, 1);
  if (temp_output_buf_) {
    tmp = temp_output_buf_;
    evbuffer_drain(tmp, evbuffer_get_length(tmp));
  }
  
  inited_ = false;
	state_ = DKCON_DISCONNECTED;
  error_ = DKCON_ERROR_NONE;
}

bool DKBaseConnection::Connect() {
  if (state_ == DKCON_CONNECTING)
    return true;

  if (!bufev_)
    return false;
  
  Reset();
  kind_ = CON_OUTGOING;
 
  bufferevent_setfd(bufev_, fd_);
	bufferevent_setcb(bufev_,
	    NULL /* EventReadCb */,
	    NULL /* EventWriteCb */,
	    EventConnectCb,
	    (void *)this);
	bufferevent_settimeout(bufev_, 0,
	    timeout_ != -1 ? timeout_ : DK_CONNECT_TIMEOUT);

  EnableWrite();

  string ip;

  /* Libevent evget_addrinfo has a bug */
  if (!DKGetHostByName(host_, ip)) {
    ConnectFail(DKCON_ERROR_ERRNO);
    return false;
  }

  if (ip.empty())
    ip = host_;

  state_ = DKCON_CONNECTING;

  int res = bufferevent_socket_connect_hostname(bufev_,
      NULL, AF_INET, ip.c_str(), port_);
  
  if (res < 0) {
    DK_DEBUG("[error] %s: bufferevent_socket_connect_hostname return %d\n",
        __func__, res);
    ConnectFail(DKCON_ERROR_ERRNO);     
    return false;
  }
 
  int fd = bufferevent_getfd(bufev_);
  if (fd != -1) {
    socklen_t namelen = sizeof(struct sockaddr_in);
    struct sockaddr_in name;
    socklen_t peer_namelen = sizeof(struct sockaddr_in);
    struct sockaddr_in peer_name;

    
    if (0 == getpeername(fd,
        (struct sockaddr *)&peer_name, &peer_namelen))
    {
      if (0 == getsockname(fd,
            (struct sockaddr *)&name, &namelen))
      {
        int flags = 1;
        if ((name.sin_addr.s_addr == peer_name.sin_addr.s_addr) &&
            (name.sin_port == peer_name.sin_port)) {
          setsockopt(fd, SOL_SOCKET, SO_REUSEADDR,
                     (void *)&flags, sizeof(flags));
          DK_DEBUG("[error] src addr == dest addr\n");
          ConnectFail(DKCON_ERROR_ERRNO);     
          return false;
        }
      }
    }
  }

  return true;
}

void DKBaseConnection::ConnectMade() {
  OnConnect();

  if (kind_ == CON_OUTGOING) {
    set_state(DKCON_IDLE);
    bufferevent_setcb(bufev_,
        EventReadCb,
        EventWriteCb,
        EventErrorCb,
        this);

    if (timeout_ != -1) {
      bufferevent_settimeout(bufev_, timeout_, timeout_);
    }

    if (temp_output_buf_ && evbuffer_get_length(temp_output_buf_) > 0) {
      AddOutputBuffer(temp_output_buf_);
    }

    if (evbuffer_get_length(get_output_buffer()) > 0)
      StartWrite();
  }
}

void DKBaseConnection::FreeIncomingConn() {
  if (incoming_conn_free_cb_)
    incoming_conn_free_cb_(this, incoming_conn_free_cb_arg_);
}
