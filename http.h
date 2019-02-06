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
#include "uri.h"
#include "http_parser.h"

#ifndef _WIN32
#include <unistd.h>
#endif

extern "C" {
#include "uv.h"
#include "http_parser.h"
}

#define MAX_WRITE_HANDLES 1000

#define ASSERT_STATUS(status, msg) \
  if (status != 0) { \
    std::cerr << msg << ": " << uv_err_name(status); \
    exit(1); \
  }

namespace http {

  using namespace std;

  template <class Type> class Buffer;
  template <class Type> class IStream;

  class Request;
  class Response;
  class ClientOrServer;
  class Server;
  class Client;
  class Context;

  extern const string CRLF;
  extern void free_context (uv_handle_t*);

  extern int parser_on_url (http_parser* parser, const char* at, size_t len);
  extern int parser_on_header_field (http_parser* parser, const char* at, size_t length);
  extern int parser_on_header_value (http_parser* parser, const char* at, size_t length);
  extern int parser_on_headers_complete (http_parser* parser);
  extern int parser_on_body (http_parser* parser, const char* at, size_t len);
  extern int parser_on_message_complete (http_parser* parser);

  static http_parser_settings parser_settings = {
    .on_url = parser_on_url,
    .on_header_field = parser_on_header_field,
    .on_header_value = parser_on_header_value,
    .on_headers_complete = parser_on_headers_complete,
    .on_body = parser_on_body,
    .on_message_complete = parser_on_message_complete,
  };

  template <class Type> 
  class Buffer : public stringbuf {
 
    friend class Request;
    friend class Response;

    Type* stream;

    Buffer<Type> (ostream& str) {};
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
      stringstream body;
      string next_header;
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
      string body = "";
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
      uv_connect_t connect_req;
      uv_write_t write_req;
      http_parser parser;
      void* instance;
  };

  class ClientOrServer {
    protected:
      uv_loop_t* UV_LOOP;
      uv_tcp_t socket_;
      static void read_allocator(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf);
      static void read(uv_stream_t* tcp, ssize_t nread, const uv_buf_t* buf);

    public:
      virtual int complete(http_parser* parser) = 0;

  };

  //extern void attachEvents(http_parser_settings& settings);

  class Client : public ClientOrServer {

    friend class Response;

    private:
      typedef function<void (
        Response& res)> Listener;

      Listener listener;
      
      void connect(uv_loop_t* loop);
      int complete(http_parser* parser);
      static void on_resolved(uv_getaddrinfo_t* req, int status, struct addrinfo* res);
      static void on_connect(uv_connect_t* req, int status);

    protected:
      uv_getaddrinfo_t addr_req;
      uv_shutdown_t shutdown_req;

    public:
       struct Options {
        string host = "localhost";
        int port = 80;
        string method = "GET";
        string url = "/";
      };

      Options opts;

      Client(Options o, Listener listener);
      Client(uv_loop_t* loop, Options o, Listener listener);
      Client(string u, Listener listener);
      Client(uv_loop_t* loop, string u, Listener listener);
      ~Client() {}
  };

  class Server : public ClientOrServer {

    friend class Response;

    private:
      typedef function<void (
        Request& req, 
        Response& res)> Listener;
 
      Listener listener;
      
      int complete(http_parser* parser);
      static void on_connect(uv_stream_t* handle, int status);

    public:
      Server (Listener listener);
      ~Server() {}
      int listen (const char*, int);
      int listen (uv_loop_t* loop, const char*, int);
  };

} // namespace http

#endif

