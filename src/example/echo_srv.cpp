
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "donkey_core.h"

#define dlog1 printf

using namespace std;

class MyServer;

static MyServer *server;

class SrvConnection: public DonkeyBaseConnection {

  virtual void ConnectedCallback() {
    dlog1("new SrvConnection fd:%d %s:%d\n", fd_, host_.c_str(), port_);
  }
  
  virtual void CloseCallback() {
    dlog1("SrvConnection close fd:%d %s:%d\n",
        fd_, host_.c_str(), port_); 
  }

  virtual void ErrorCallback() {
    dlog1("SrvConnection Error %s\n", this->get_error_string());
  }

  virtual void WriteCallback() {
    dlog1("SrvConnection %s\n", __func__);

  }

  virtual enum READ_STATUS ReadCallback() {
    struct evbuffer *inbuf = get_input_buffer();
    size_t inbuf_size = evbuffer_get_length(inbuf);
    
    dlog1("ReadCallback %d\n", inbuf_size);
    if (inbuf_size <= 0)
      return READ_BAD_CLIENT;
    
    char buf[1024];
    int nread = evbuffer_remove(inbuf, buf, sizeof(buf));
    
    if (nread > 0) {
      
      if (strncmp(buf, "quit", 4) == 0) {
        /* close connection after response */
        set_keep_alive(false);
      } else {
        set_keep_alive(true);
      }
      Send(buf, nread);
    } else
      return READ_BAD_CLIENT;
    
    return READ_ALL_DATA;
  }
};

class MyServer: public DonkeyServer {
  virtual void ClockCallback() {
  }

  virtual DonkeyBaseConnection *NewConnection() {
    return new SrvConnection(); 
  }

  virtual void ConnectionMade(DonkeyBaseConnection *conn) {
  }
};

#define PORT 60006

int main(int argc, char **argv) {
  server = new MyServer();

  if (!server || !server->Init()) {
    dlog1("new DonkeyServer Init error\n");
    return 1;
  }
  
  dlog1("server start ......\n");

  signal(SIGPIPE, SIG_IGN);
  signal(SIGHUP, SIG_IGN);

  if (!server->StartListenTCP(NULL, PORT, 1024))
    return 1;

  server->set_timeout(10);
  server->EventLoop();

  delete server;
}
