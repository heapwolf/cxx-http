#ifndef HTTP_UV_H
#define HTTP_UV_H

#include <map>
#include <vector>
#include <string>
#include <sstream>
#include <iostream>
#include <unistd.h>
#include <functional>

#include "uv.h"
#include "http_parser.h"

#ifndef _WIN32
#include <unistd.h>
#endif

extern "C" {
#include "uv.h"
#include "http_parser.h"
}

#define MAX_WRITE_HANDLES 1000

#ifndef HTTP_UV_LOOP
#define HTTP_UV_LOOP
static uv_loop_t *UV_LOOP;
#endif

namespace http {

using namespace std;

template <class Type> class Buffer;
template <class Type> class IStream;

class Request;
class Response;
class Server;
class Client;
class ServerEvents;
class ClientEvents;


template <class Type> class Buffer : public stringbuf {

  friend class Request;
  friend class Response;

  Type *stream;

  Buffer<Type> (ostream &str) {};
  ~Buffer () {};
  virtual int sync () {

    string out = str();
    std::ostringstream buf;
    buf << out;
    out = buf.str();
    stream->writeOrEnd(out, false);
    buf.flush();
    str("");
    return 0;
  }
};


template <class Type> class IStream : virtual public ostream {

  public:
    IStream () { };
};


class Request {
  public:
    string url;
    string method;
    string status_code;
    string body;
    map<const string, const string> headers; 

    Request() {}
    ~Request() {}
};


class ServerEvents {
  public:

    typedef function<void (
      Request &req, 
      Response &res)> Listener;

    Listener listener;

    ServerEvents(Listener fn);
    ~ServerEvents() {}
};


class Response : public IStream<Response> {

  friend class Buffer<class Response>;

  stringstream stream;
  Buffer<Response> buffer;
  ServerEvents::Listener listener;

  void writeOrEnd(string, bool);

  int write_count = 0; 
  bool writtenOrEnded = false;
  bool ended = false;
  bool headersSet = false;
  bool statusSet = false;
  bool contentLengthSet = false;

  public:
 
    http_parser parser;
    
    int statusCode = 200;
    string statusAdjective = "OK";
    map<const string, const string> headers;

    void setHeader (const string, const string);
    void setStatus (int);
    void setStatus (int, string);
    
    void write (string);
    void end (string);
    void end ();

    Response(ServerEvents::Listener fn) :
      IStream(), 
      ostream(&buffer), 
      buffer(stream) {
        buffer.stream = this;
      }
    ~Response() {
    }
};

/* inline Response &operator << (Response &res, string s) {
  res.write(s);
  return res;
}*/

class Client : public Request {
  public:
    map<int, uv_write_t> writes;
    //uv_write_t write_req;
    uv_tcp_t handle;
    http_parser parser;
};

class Server {

  friend class Response;

  private:

  protected:
    uv_tcp_t socket_;
    ServerEvents events;

  public:
    static void free_client (uv_handle_t *);
    Server (ServerEvents::Listener listener) :
      events(listener) {}
    int listen (const char *, int);
};


} // namespace http

#endif

