
#include "http.h"

namespace http {

  using namespace std;

  void free_context (uv_handle_t *handle) {
    auto *context = reinterpret_cast<Context*>(handle->data);
    context->writes.clear();
    free(context);
  }

  //
  // Events
  //
  template <class T>
  void attachEvents(T *instance) {

    // http parser callback types
    static function<int(http_parser *parser)> on_message_complete;

    // called once a connection has been made and the message is complete.
    on_message_complete = [&](http_parser *parser) -> int {
      return instance->complete(parser);
    };

    // called after the url has been parsed.
    instance->settings.on_url =
      [](http_parser *parser, const char *at, size_t len) -> int {
        Context *context = static_cast<Context *>(parser->data);
        if (at && context) { context->url = string(at, len); }
        return 0;
      };

    // called when there are either fields or values in the request.
    instance->settings.on_header_field =
      [](http_parser *parser, const char *at, size_t length) -> int {
        return 0;
      };

    // called when header value is given
    instance->settings.on_header_value =
      [](http_parser *parser, const char *at, size_t length) -> int {
        return 0;
      };

    // called once all fields and values have been parsed.
    instance->settings.on_headers_complete =
      [](http_parser *parser) -> int {
        Context *context = static_cast<Context *>(parser->data);
        context->method = string(http_method_str((enum http_method) parser->method));
        return 0;
      };

    // called when there is a body for the request.
    instance->settings.on_body =
      [](http_parser *parser, const char *at, size_t len) -> int {
        Context *context = static_cast<Context *>(parser->data);
        if (at && context) { context->body = string(at, len); }
        return 0;
      };

    // called after all other events.
    instance->settings.on_message_complete =
      [](http_parser *parser) -> int {
        return on_message_complete(parser);
      };
  
  }

  //
  // Response.
  //
  void Response::setHeader (const string key, const string val) {
    headersSet = true;
    if (writtenOrEnded) throw runtime_error("Can not set headers after write");

    if (key == "Content-Length") {
      contentLengthSet = true;
    }
    headers.insert({ key, val });
  }

  void Response::setStatus (int code) {
    
    statusSet = true;
    if (writtenOrEnded) throw runtime_error("Can not set status after write");
    statusCode = code;
  }

  void Response::setStatus (int code, string ad) {

    statusSet = true;
    if (writtenOrEnded) throw runtime_error("Can not set status after write");
    statusCode = code;
    statusAdjective = ad;
  }

  void Response::writeOrEnd(string str, bool end) {

    if (ended) throw runtime_error("Can not write after end");

    if (!writtenOrEnded) {

      stringstream ss;
      ss << "HTTP/1.1 " << statusCode << " " << statusAdjective << "\r\n";

      for (auto &header : headers) {
        ss << header.first << ": " << header.second << "\r\n";
      }

      str = ss.str() + "\r\n\r\n" + str;
      writtenOrEnded = true;
      ss.str("");
    } 

    // response buffer
    uv_buf_t resbuf = {
      .base = (char *) str.c_str(),
      .len = str.size()
    };

    Context *context = static_cast<Context *>(this->parser.data);

    auto id = write_count++;

    uv_write_t write_req;
    context->writes.insert({ id, write_req });

    if (end) {

      ended = true;

      uv_write(&context->writes.at(id), (uv_stream_t*) &context->handle, &resbuf, 1,
        [](uv_write_t *req, int status) {
          if (!uv_is_closing((uv_handle_t*) req->handle)) {
            uv_close((uv_handle_t*) req->handle, free_context);
          }
        }
      );
    }
    else {
      uv_write(&context->writes.at(id), (uv_stream_t*) &context->handle, &resbuf, 1, NULL);
    }
  }

  void Response::write(string s) {
    this->writeOrEnd(s, false);
  }

  void Response::end(string s) {
    this->writeOrEnd(s, true);
  }

  void Response::end() {
    this->writeOrEnd("", true);
  }


  //
  // Client
  //
  Client::Client (string u, Listener fn) {
    //url = u;
    listener = fn;
    attachEvents<Client>(this);
  }

  Client::Client (Client::Options o, Listener fn) {
    opts = o;
    listener = fn;
    attachEvents<Client>(this);
  }

  int Client::complete(http_parser *parser) {
    Context *context = reinterpret_cast<Context*>(parser->data);

    Response res;
    res.parser = *parser;

    listener(res);
    return 0;
  }

  //
  // Server
  //
  Server::Server (Listener fn) {
    listener = fn;
    attachEvents<Server>(this);
  }

  int Server::complete (http_parser *parser) {
    Context *context = reinterpret_cast<Context*>(parser->data);
    Request req;
    Response res;

    req.url = context->url;
    req.method = context->method;
    res.parser = *parser;

    listener(req, res);
    return 0;
  }

  int Server::listen (const char *ip, int port) {

    #ifdef _WIN32
      SYSTEM_INFO sysinfo;
      GetSystemInfo( &sysinfo );
      int cores = sysinfo.dwNumberOfProcessors;
    #else
      int cores = sysconf(_SC_NPROCESSORS_ONLN);
    #endif

    std::stringstream cores_string;
    cores_string << cores;

    #ifdef _WIN32
      SetEnvironmentVariable("UV_THREADPOOL_SIZE", cores_string);
    #else
      setenv("UV_THREADPOOL_SIZE", cores_string.str().c_str(), 1);
    #endif

    struct sockaddr_in address;

    static function<void(uv_stream_t *socket, int status)> on_connect;
    static function<void(uv_stream_t *tcp, ssize_t nread, const uv_buf_t *buf)> read;

    UV_LOOP = uv_default_loop();
    uv_tcp_init(UV_LOOP, &socket_);

    //
    // @TODO - Not sure exactly how to use this,
    // after the initial timeout, it just
    // seems to kill the server.
    //
    //uv_tcp_keepalive(&socket_,1,60);

    uv_ip4_addr(ip, port, &address);
    uv_tcp_bind(&socket_, (const struct sockaddr*) &address, 0);

    // called once a connection is made.
    on_connect = [&](uv_stream_t *handle, int status) {
      Context *context = new Context();

      // init tcp handle
      uv_tcp_init(UV_LOOP, &context->handle);

      // init http parser
      http_parser_init(&context->parser, HTTP_REQUEST);

      // client reference for parser routines
      context->parser.data = context;

      // client reference for handle data on requests
      context->handle.data = context;

      // accept connection passing in refernce to the client handle
      uv_accept(handle, (uv_stream_t*) &context->handle);

      // called for every read
      read = [&](uv_stream_t *tcp, ssize_t nread, const uv_buf_t *buf) {
        ssize_t parsed;
        Context *context = static_cast<Context *>(tcp->data);

        if (nread >= 0) {
          parsed = (ssize_t) http_parser_execute(&context->parser,
                                                 &settings,
                                                 buf->base,
                                                 nread);

          // close handle
          if (parsed < nread) {
            uv_close((uv_handle_t*) &context->handle, free_context);
          }
        } else {
          if (nread != UV_EOF) {
            // @TODO - debug error
          }

          // close handle
          uv_close((uv_handle_t*) &context->handle, free_context);
        }

        // free request buffer data
        free(buf->base);
      };

      // allocate memory and attempt to read.
      uv_read_start((uv_stream_t*) &context->handle,
          // allocator
          [](uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf) {
            *buf = uv_buf_init((char*) malloc(suggested_size), suggested_size);
          },

          // reader
          [](uv_stream_t *tcp, ssize_t nread, const uv_buf_t *buf) {
            read(tcp, nread, buf);
          });
    };

    uv_listen((uv_stream_t*) &socket_, MAX_WRITE_HANDLES,
        // listener
        [](uv_stream_t *socket, int status) {
          on_connect(socket, status);
        });

    // init loop
    uv_run(UV_LOOP, UV_RUN_DEFAULT);
    return 0;
  }
}

