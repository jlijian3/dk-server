/**
 * author:lijian2@ucweb.com
 * date:2012-03
 */

#ifndef __DONKEY_HTTP_INCLUDE__
#define __DONKEY_HTTP_INCLUDE__

#include "donkey_common.h"

class DonkeyHttpClient;

class DonkeyHttpRequest {
public:
  DonkeyHttpRequest() : http_req_(NULL), arg_(NULL) {
  }

  virtual ~DonkeyHttpRequest() {
  }

  bool Init(void *arg=NULL) {
    http_req_ = evhttp_request_new(EventHttpRequestCb, (void *)this);
    arg_ = arg;
    return http_req_ != NULL; 
  }

  bool AddHeader(const char *key,
                 const char *value) {
    assert(http_req_);
    return 0 == evhttp_add_header(evhttp_request_get_output_headers(http_req_),
      key, value);  
  }
  
  bool AddPostData(const void *data, size_t len) {
    assert(http_req_);
    return 0 == evbuffer_add(evhttp_request_get_output_buffer(http_req_), data, len); 
  }

  struct evhttp_request *get_http_req() {
    return http_req_;
  }

  virtual void HandleResponse(struct evhttp_request *req, void *arg);
  void DoHandleResponse(struct evhttp_request *req) {
    HandleResponse(req, arg_);
  }
  
  void DebugResponse(struct evhttp_request *req);
  void DebugHeaders(struct evkeyvalq *headers);
  void DebugRequest(struct evhttp_request *req);

private:
  static void EventHttpRequestCb(struct evhttp_request *req, void *arg);

protected:
  struct evhttp_request *http_req_;
  void *arg_;
};

class DonkeyHttpClient {
public: 
  DonkeyHttpClient() : last_conn_idx_(0) {
  }
  
  virtual ~DonkeyHttpClient() {
    for (size_t i = 0; i < http_conns_.size(); i++) {  
      if (http_conns_[i])
        evhttp_connection_free(http_conns_[i]);
    }
  }

  bool Init(struct event_base *evbase,
            const char *host,
            unsigned short port,
            int conns=10); 

  DonkeyHttpRequest *NewRequest() {
    DonkeyHttpRequest *req = new DonkeyHttpRequest();
    if (req && !req->Init()) {
      delete req;
      req = NULL;
    }

    return req;
  }

  struct evhttp_connection *get_http_conn() {
    if (http_conns_.empty())
      return NULL;

    int idx = last_conn_idx_ % http_conns_.size();
    last_conn_idx_ = idx + 1;
    return http_conns_[idx];
  }

  void SetTimeout(int timeout_in_secs) {
    for (size_t i = 0; i < http_conns_.size(); i++) {
      evhttp_connection_set_timeout(http_conns_[i], timeout_in_secs);
    }
  }

  void SetRetries(int retries) {
    for (size_t i = 0; i < http_conns_.size(); i++) {
      evhttp_connection_set_retries(http_conns_[i], retries);
    }
  }

  /* defined in <event2/http.h>
   * enum evhttp_cmd_type {
        EVHTTP_REQ_GET     = 1 << 0,
        EVHTTP_REQ_POST    = 1 << 1,
        EVHTTP_REQ_HEAD    = 1 << 2,
        EVHTTP_REQ_PUT     = 1 << 3,
        EVHTTP_REQ_DELETE  = 1 << 4,
        EVHTTP_REQ_OPTIONS = 1 << 5,
        EVHTTP_REQ_TRACE   = 1 << 6,
        EVHTTP_REQ_CONNECT = 1 << 7,
        EVHTTP_REQ_PATCH   = 1 << 8
    };
    
    dk_http_req will be auto free after response
  */
  bool SendRequest(DonkeyHttpRequest *dk_http_req,
                  enum evhttp_cmd_type cmd_type,
                  const char *uri) {
    struct evhttp_connection *conn = get_http_conn();
    if (!conn || !dk_http_req || !uri)
      return false;
    
    return 0 == evhttp_make_request(
        conn, dk_http_req->get_http_req(), cmd_type, uri);
  }


  virtual void CloseCallback(struct evhttp_connection *conn) {}

private:
  static void EventHttpCloseCb(struct evhttp_connection *conn, void *arg);

protected:
  std::vector<struct evhttp_connection *> http_conns_;
  std::string               host_;
  unsigned short            port_;
  int                       last_conn_idx_; 
};

#endif
