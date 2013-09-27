/**
 * author:lijian2@ucweb.com
 * date:2012-03
 */

#include "dk_core.h"
using namespace std;


/*********** DKHttpRequest ************/

void DKHttpRequest::HandleResponse(struct evhttp_request *req, void *arg) {
  DebugResponse(req);
}

void DKHttpRequest::EventHttpRequestCb(
    struct evhttp_request *req, void *arg) {
  DKHttpRequest *http_req = (DKHttpRequest *)arg;
  
  if (http_req) {
    http_req->DoHandleResponse(req);
    delete http_req; 
  }
}

void DKHttpRequest::DebugResponse(struct evhttp_request *req) {
  if (!req)
    return;
  
  struct evkeyvalq    *headers;
  struct evbuffer     *input_buffer;
  const char *         value; 
  int                  content_length = -1;
  string               s_headers = "headers:\n";
  string               body;
 
  DK_DEBUG(">>>>>>>>>>>\n");
  DK_DEBUG("Http status %d\n", evhttp_request_get_response_code(req));
  headers = evhttp_request_get_input_headers(req);
  DebugHeaders(headers);

  value = evhttp_find_header(headers, "Content-Length");
  if (value)
    content_length = atoi(value);

  input_buffer = evhttp_request_get_input_buffer(req); 
  
  if (input_buffer) {
    const char *tmp = (const char *)evbuffer_pullup(input_buffer, content_length); 
    if (tmp)
      body.assign(tmp, content_length); 
    DK_DEBUG("Http body: %.*s\n", body.size(), body.data());
  }
  
  DK_DEBUG("<<<<<<<<<<<\n"); 
}

void DKHttpRequest::DebugRequest(struct evhttp_request *req) {
  if (!req)
    return; 
  struct evkeyvalq *headers = evhttp_request_get_output_headers(req);
  DK_DEBUG(">>>>>>>>>>>\n"); 
  DebugHeaders(headers);
  DK_DEBUG("<<<<<<<<<<<\n");
}

void DKHttpRequest::DebugHeaders(struct evkeyvalq *headers) {
  struct evkeyval *header;
  string s_headers = "Http headers:\n";
  
  if (!headers)
    return;
   
  for (header = headers->tqh_first; header;
    header = header->next.tqe_next) {
    s_headers += header->key;
    s_headers += ":";
    s_headers += header->value;
    s_headers += "\n";
  }
  
  DK_DEBUG("%.*s\n", s_headers.size(), s_headers.data());
}


/*********** DKHttpClient ************/

bool DKHttpClient::Init(
    struct event_base *evbase, const char *host, unsigned short port, int conns) {
  if (!http_conns_.empty())
    return false;

  if (!evbase || !host || port == 0)
    return false;

  host_ = host;
  port_ = port;

  string ip;

  /* Libevent evget_addrinfo has a bug */
  DKGetHostByName(host, ip);
  if (ip.empty())
    ip = host;

  if (conns <=0)
    conns = 10;

  for (int i = 0; i < conns; i++) {
    struct evhttp_connection * conn =
        evhttp_connection_base_new(evbase, NULL, ip.c_str(), port);
    if (!conn)
      return false;

    http_conns_.push_back(conn);
    evhttp_connection_set_closecb(conn, EventHttpCloseCb, this);
  }

  return true;
}

void DKHttpClient::EventHttpCloseCb(struct evhttp_connection *conn, void *arg) {
  DKHttpClient *http_client = (DKHttpClient *)arg;
  
  if (http_client) 
    http_client->OnClose(conn);
}


