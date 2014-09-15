#ifndef NODEUV_HTTP_H
#define NODEUV_HTTP_H

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
class Context;

static void free_context (uv_handle_t *);

template <class Type> 
class Buffer : public stringbuf {

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
    stream->writeOrEnd(out, true);
    buf.flush();
    str("");
    return 0;
  }
};


template <class Type> 
class IStream : virtual public ostream {

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


class Response : public IStream<Response> {

  friend class Buffer<class Response>;
  friend class Server;

  stringstream stream;
  Buffer<Response> buffer;

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

    Response() :
      IStream(), 
      ostream(&buffer), 
      buffer(stream) {
        buffer.stream = this;
      }
    ~Response() {
    }
};

/*
  // @TODO
  // Maybe have each op call write
  //
  inline Response &operator << (Response &res, string s) {
  res.write(s);
  return res;
}*/


class Context : public Request {

  public:
    map<int, uv_write_t> writes;
    uv_tcp_t handle;
    http_parser parser;
};


class Client {

  template<typename Type>
  friend void attachEvents(Type *instance);
  friend class Response;

  private:
     typedef function<void (
      Response &res)> Listener;

    Listener listener;
    http_parser_settings settings;
    int complete(http_parser *parser); 
    uv_tcp_t socket_;

  protected:
    uv_connect_t connect_req;
    uv_shutdown_t shutdown_req;
    uv_write_t write_req;

    struct Options {
      string host;
      int port;
    };

    Options opts;

  public:
    uv_tcp_t handle;
    http_parser parser;

    Client(Options o, Listener listener);
    Client(string u, Listener listener);
    ~Client() {}
};

class Server {

  template<typename Type>
  friend void attachEvents(Type *instance);
  friend class Response;

  private:
    typedef function<void (
      Request &req, 
      Response &res)> Listener;
  
    Listener listener;
    http_parser_settings settings;
    int complete(http_parser *parser); 
    uv_tcp_t socket_;

  public:
    Server (Listener listener);
    int listen (const char *, int);
};


} // namespace http


#endif

//Client client("http://google.com", [](&res) {

  

//});

// client.write("");
// client.end();
