/**
 * author:jlijian3@gmail.com
 * date:2012-03-08
 */

#ifndef __DONKEY_SERVER_INCLUDE__
#define __DONKEY_SERVER_INCLUDE__

#include "dk_common.h"
#include "dk_ev_thread.h"

struct event_base;
struct evconnlistener;
struct event;
struct sockaddr_in;

class DKBaseConnection;

class DKBaseServer : public DKEventThread {
public:
  enum {FREE_CONNS = 200};
  DKBaseServer(); 
  virtual ~DKBaseServer();

  bool Init();

  bool StartListenTCP(const char *address,
                      unsigned short port,
                      int backlog);
 
  int EventLoop();

  bool MakeConnection(int fd, const char *host, unsigned short port);  

  void FreeConn(DKBaseConnection *conn);

  DKBaseConnection *get_conn(int conn_id);

  int get_conns_map_size() {
    return conns_map_.size();
  }

  int get_free_conns_size() {
    return free_conns_.size();
  }
  
  unsigned int get_current_time() {
    return current_time_;
  }

  void set_timeout(int timeout_in_secs) {
    timeout_ = timeout_in_secs;
  }

protected:
  virtual void ClockCallback() {
  }

  virtual DKBaseConnection *NewConnection();

  virtual void ConnectionMade(DKBaseConnection *conn) {
  }

private:
  static void IncomingConnFreeCallback(DKBaseConnection *conn, void *arg);

private:
  static void ListenerCallback(struct evconnlistener *listener,
                               int fd,
                               struct sockaddr *sa,
                               int socklen,
                               void *arg);

  static void ClockHandler(int fd, short which, void *arg);

private:
  /*struct event_base             *base_;*/
	struct evconnlistener         *listener_;
	struct event                  *signal_event_;

	struct sockaddr_in             sin_;
  std::vector<DKBaseConnection *> free_conns_;
  static unsigned int            current_time_;
  std::map<int, DKBaseConnection *> conns_map_;
  int timeout_;
};

#endif
