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
  static void free_client (uv_handle_t *);

  /**
   * `http' api
   */

  template <class Type> class Buffer;
  template <class Type> class IStream;
  class Response;
  class Request;
  class Client;
  class Server;

  /**
   * `Baton' class
   */

  class Baton {
    public:
      Client *client;
      string result;
      uv_work_t request;
      bool error;
  };

  /**
   * `Buffer' class
   *
   * Extends `std::stringbuf'
   */

  template <class Type> class Buffer : public stringbuf {

    friend class Request;
    friend class Response;

    /**
     * Pointer to buffer stream
     */

    Type *stream_;

    public:

    /**
     * Write callback type attached to `stream_'
     */

    typedef function<void (string s)> WriteCallback;

    /**
     * `Buffer<Type>' constructor
     */

    Buffer<Type> (ostream &str) {};

    /**
     * `Buffer<Type>' destructor
     */

    ~Buffer () {};

    /**
     * Implements the `std::streambuf::sync()' virtual
     * method. Returns `0' on success, otherwise `1' on
     * failure.
     */

    virtual int sync () {
      // create out stream
      string out = str();

      // write buffer stream
      std::ostringstream buf;

      // concat
      buf << out;

      // write
      out = buf.str();

      // defer to end callback
      stream_->writeOrEnd(out, false);
      buf.flush();
      str("");
      return 0;
    }

  };


  /**
   * `IStream' interface
   */

  template <class Type> class IStream : virtual public ostream {

    protected:

      /**
       * `IStream' constructor
       */

    public:
      IStream () { };

      void write (string);
      void read () {};
      void end () {};
  };

  /**
   * `Request' class
   */

  class Request {
    public:

      /**
       * Request URL
       */

      string url;

      /**
       * Request HTTP method
       */

      string method;

      /**
       * Request HTTP status code
       */

      string status_code;

      /**
       * Request body
       */

      string body;

      /**
       * Request headers map
       */

      map<const string, const string> headers;

  };

  /**
   * `Client' class
   *
   * Extends `http::Request'
   */

  class Client : public Request {
    public:
      /**
       * uv tcp socket handle
       */

      uv_tcp_t handle;

      /**
       * http parser instance
       */

      http_parser parser;

      /**
       * uv write worker
       */

      //uv_write_t write_req;
  };

  /**
   * `Response' stream class
   *
   * Extends `std::ostream'
   */

  class Response : public IStream<Response> {

    friend class Buffer<class Response>;

    private:

      /**
       * Response buffer
       */

      Buffer<Response> buffer_;

      /**
       * `Buffer' string stream
       */

      stringstream stream_;

      /**
       * Used to determine if a write has been made
       * or if the end method has been called.
       */

      void writeOrEnd(string, bool);

      bool writtenOrEnded = false;
      bool ended = false;
      bool headersSet = false;
      bool statusSet = false;
      bool contentLengthSet = false;

    protected:

      /**
       * Stream sync interface
       */

      //virtual int sync (ostringstream &, size_t);
      int write_count = 0;

    public:

      map<int, uv_write_t> writes;

      http_parser parser;
      /**
       * `Buffer::WriteCallback' detect predicate
       */

      bool hasCallback = false;

      /**
       * Sets a key value pair as a HTTP
       * header
       */

      void setHeader (const string, const string);

      /**
       * Sets the HTTP status code
       */

      string statusAdjective = "OK";
      int statusCode = 200;

      void setStatus (int);
      void setStatus (int, string);

      /**
       * Key to value map of response headers
       */

      map<const string, const string> headers;

      /**
       * Write to buffer stream
       */

      void write (string);
      void end (string);
      void end ();

      /**
       * `Response' constructor
       *
       * Initializes super class and `Buffer buffer_'
       * instance with `stringstream stream_'
       */

      Response () : IStream(), ostream(&buffer_), buffer_(stream_) {
        buffer_.stream_ = this;
      }

      /**
       * `Response' destructor
       */

      ~Response() { 
      };
  };

  /**
   * `Events' class
   */

  class Events {

    public:
 
      /**
       * `Listener' callback
       */

      typedef function<void (Request &, Response &)> Listener;

      /**
       * HTTP event listener callback
       */

      Listener listener;

      /**
       * `Events' constructor
       */

      Events(Listener fn);
  };

  /**
   * `Server' class
   */

  class Server {

    protected:

      /**
       * Internal uv tcp socket
       */

      uv_tcp_t socket_;

      /**
       * Internal server events
       */

      Events events_;

    public:

      /**
       * `Server' constructor
       */

      Server (Events::Listener listener) : events_(listener) {}

      /**
       * Called by the user when they want to start listening for
       * connections. starts the main event loop, provides parser, etc.
       */

      int listen (const char *, int);
  };

  static void free_client (uv_handle_t *handle) {
    auto *client = reinterpret_cast<Client*>(handle->data);
    free(client);
  }

} // namespace http

#endif

