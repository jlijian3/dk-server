/**
 * author:lijian2@ucweb.com
 * date:2012-03-08
 */

#include "dk_core.h"
#include "dk_log.h"

using namespace std;

log_func_t DKLog::log_func_ = NULL;
FILE *DKLog::fstream_ = NULL;

unsigned int DKBaseServer::current_time_;

DKBaseServer::DKBaseServer()
    : listener_(NULL),
      signal_event_(NULL),
      timeout_(-1) {
}

DKBaseServer::~DKBaseServer() {
  if (listener_)
    evconnlistener_free(listener_);
  if (signal_event_)
    event_free(signal_event_);
}

bool DKBaseServer::Init() {
  DKEventThread::Init();
  ClockHandler(0, 0, this);
  return true;
}

bool DKBaseServer::StartListenTCP(const char *address,
                                  unsigned short port,
                                  int backlog) {
  if (!base_)
    return false;

  if (listener_)
    return false;

  memset(&sin_, 0, sizeof(sin_));
	sin_.sin_family = AF_INET;
	sin_.sin_port = htons(port);
  if (address)
    inet_aton(address, &sin_.sin_addr);

	listener_ = evconnlistener_new_bind(base_, ListenerCallback,
      (void *)this, LEV_OPT_REUSEABLE|LEV_OPT_CLOSE_ON_FREE,
      backlog, (struct sockaddr*)&sin_, sizeof(sin_));

	if (!listener_) {
    DK_DEBUG("[error] %s %s\n", __func__, strerror(errno));
		return false;
	}
  
  return true;
}

int DKBaseServer::EventLoop() {
  return ThreadRoutine();
}

bool DKBaseServer::MakeConnection(int fd,
                                  const char *host,
                                  unsigned short port) {
  DKBaseConnection *conn = NULL;  
  if (free_conns_.empty()) {
    conn = NewConnection();
  } else {
    conn = free_conns_.back();
    free_conns_.pop_back();
    DK_DEBUG("%s get free conn %x\n", __func__, conn);
  }

  
  if (!conn) {
    evutil_closesocket(fd);    
    return false;
  }
  
  if (!conn->Init(base_, fd, host, port)) {
    evutil_closesocket(fd); 
    delete conn;
    return false;
  }

  map<int, DKBaseConnection *>::iterator it = conns_map_.find(conn->get_id());
  if (it != conns_map_.end() && it->second != NULL) {
    DKBaseConnection *old_conn = it->second;
    DK_DEBUG("[error] %s: conn_id %d fd %d already exists in conns_map_\n",
             __func__, old_conn->get_id(), old_conn->get_fd());
    conn->Reset();
    delete conn;
    return false;
  } else
    conns_map_[conn->get_id()] = conn;

  conn->set_incoming_conn_free_cb(IncomingConnFreeCallback, this);

  ConnectionMade(conn);

  conn->ConnectMade();
  
  conn->set_timeout(timeout_);
  return conn->StartRead();
}

void DKBaseServer::ListenerCallback(struct evconnlistener *listener,
                                    evutil_socket_t fd,
                                    struct sockaddr *sa,
                                    int salen,
                                    void *arg) {
  DKBaseServer *server = (DKBaseServer *)arg;
  assert(server);

  char ntop[NI_MAXHOST];
	char strport[NI_MAXSERV];  
  
  int ret = getnameinfo(sa, salen,
                        ntop, sizeof(ntop), strport, sizeof(strport),
                        NI_NUMERICHOST|NI_NUMERICSERV);
  if (ret != 0)
    perror("getnameinfo");

  if (!server->MakeConnection(fd, ntop, atoi(strport)))
    DK_DEBUG("[error] %s MakeConnection failed\n", __func__);
}

void DKBaseServer::ClockHandler(int fd, short which, void *arg) {
  static struct event clockevent;
  static bool initialized = false;
   
  struct timeval t;
  DKBaseServer *server = (DKBaseServer *)arg;
  assert(server);
  assert(server->get_base());

  if (initialized) {
    /* only delete the event if it's actually there. */
    evtimer_del(&clockevent);
  } else {
    initialized = true;
  }

  struct timeval timer;

  gettimeofday(&timer, NULL);
  current_time_ = (unsigned int)timer.tv_sec;

  server->ClockCallback();

  t.tv_sec = 1;
  t.tv_usec = 0;
  evtimer_set(&clockevent, ClockHandler, arg);
  event_base_set(server->get_base(), &clockevent);
  evtimer_add(&clockevent, &t);
}

void DKBaseServer::IncomingConnFreeCallback(
    DKBaseConnection *conn, void *arg) {
  DKBaseServer *me = (DKBaseServer *)arg;
  if (me)
    me->FreeConn(conn);
}

void DKBaseServer::FreeConn(DKBaseConnection *conn) {
  if (!conn)
    return;

 
  map<int, DKBaseConnection *>::iterator it = conns_map_.find(conn->get_id());

  if (it == conns_map_.end()) {
    DK_DEBUG("[error] %s: conn_id %d fd %d not exists in conns_map_\n",
             __func__, conn->get_id(), conn->get_fd());
  } else
    conns_map_.erase(conn->get_id());

  if (free_conns_.size() > FREE_CONNS)
    delete conn;
  else
    free_conns_.push_back(conn);
}

DKBaseConnection *DKBaseServer::get_conn(int conn_id) {
  map<int, DKBaseConnection *>::iterator it = conns_map_.find(conn_id);
  if (it != conns_map_.end())
    return it->second;
  else
    return NULL;
}

DKBaseConnection *DKBaseServer::NewConnection() {
  return new DKBaseConnection(); 
}
