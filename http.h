#ifndef NODEUV_HTTP_H
#define NODEUV_HTTP_H

#include <map>
#include <vector>
#include <string>
#include <sstream>
#include <iostream>
//#include <unistd.h>
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
  class Server;
  class Client;
  class Context;

  extern const string CRLF;
  extern void free_context (uv_handle_t*);

  template <class Type> 
  extern void attachEvents(Type* instance, http_parser_settings& settings);

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
  class IStream : public std::ostream {

    public:
      IStream (stringbuf* sb) :
		  ostream{ sb }  { };
  };


  class Request {
    public:
      string url;
      string method;
      string status_code;
      stringstream body;
      map<const string, const string> headers; 
	  uri::url url_decoded;

      Request()
      {
      }
      ~Request() {}

	  void set_url(string src)
      {
		  url = src;
		  url_decoded = uri::ParseHttpUrl(src);
      }
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
        IStream(&buffer),
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
  };


  class Client {

    template<typename Type>
    friend void attachEvents(Type* instance, http_parser_settings& settings);
    friend class Response;

    private:
      typedef function<void (
        Response& res)> Listener;

      Listener listener;
      uv_loop_t* UV_LOOP;
      uv_tcp_t socket_;
      
      void connect();
      int complete(http_parser* parser, Listener fn); 
      void on_connect(uv_connect_t* req, int status);

    protected:
      uv_getaddrinfo_t addr_req;
      uv_shutdown_t shutdown_req;

    public:
       struct Options {
		   using header = pair<string, string>;
		   using headers = vector<header>;

		   Options() {}

		   Options(string const& in_host,
			   uint16_t in_port,
			   string const& in_method,
			   string const& in_url) :
			   host(in_host),
			   port(in_port),
			   method(in_method),
			   url(in_url) {}

			string host;
			int port;
			string method = "PUT";
			string url = "/";
			headers http_headers;
      };

      Options opts;
	  string req_body;
	  uv_loop_t* uv_loop;

      Client(Options o, Listener listener);
	  Client(Options o, string req_body, Listener listener, uv_loop_t* uv_loop_);
	  Client(string u, Listener listener);
      ~Client() {}
  };

  class Server {

    template<typename Type>
    friend void attachEvents(Type* instance, http_parser_settings& settings);
    friend class Response;

  public:
	typedef function<void(
		Request& req,
		Response& res)> Listener;

  private:
      Listener listener;
      uv_loop_t* UV_LOOP;
      uv_tcp_t socket_;
      
      int complete(http_parser* parser, Listener fn);

    public:
      Server (Listener listener);
      ~Server() {}
      int listen (uv_loop_t*,const char*, int);
	  int listen (const char*, int);
  };

} // namespace http

#endif

