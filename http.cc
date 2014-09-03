
#include "http.h"

namespace http {

  /**
   * `http::Response' implementation
   */

  void Response::onEnd (Buffer<Response>::WriteCallback cb) {
    hasCallback = true;
    write_ = cb;
  }

  void Response::write (string buf) {
    *this << buf;
  }

  void Response::end () {
    *this << std::endl;
  }

  void Response::setHeader (const string key, const string val) {
    headers.insert({ key, val });
  }

  void Response::setStatus (int code) {
    statusCode = code;
  }

  int Response::sync (ostringstream &buf, size_t size) {
    // fail if callback not set
    if (!hasCallback) {
      return 1;
    }

    // write status code and status adjective
    // @TODO - write routine to determine status
    // adjective. 'OK' is hardcoded here..
    buf << "HTTP/1.1 " << statusCode << " OK\r\n";

    // write headers
    for (auto &h: headers) {
      buf << h.first << ": " << h.second << "\r\n";
    }

    // set the content length and content
    buf << "Content-Length: " << size;

    // write body
    buf << "\r\n\r\n";
    return 0;
  }

  /**
   * `Server' implementation
   */

  int Server::listen (const char *ip, int port) {
    int cores = sysconf(_SC_NPROCESSORS_ONLN);
    std::stringstream cores_string;
    //char cores_string[10];
    struct sockaddr_in address;
    static function<void(uv_stream_t *socket, int status)> on_connect;
    static function<void(uv_stream_t *tcp, ssize_t nread, const uv_buf_t *buf)> read;

    cores_string << cores;
    //sprintf(cores_string, "%d", cores);
    setenv("UV_THREADPOOL_SIZE", cores_string.str().c_str(), 1);

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
      Client *client = new Client();

      // init tcp handle
      uv_tcp_init(UV_LOOP, &client->handle);

      // init http parser
      http_parser_init(&client->parser, HTTP_REQUEST);

      // client reference for parser routines
      client->parser.data = client;

      // client reference for handle data on requests
      client->handle.data = client;

      // accept connection passing in refernce to the client handle
      uv_accept(handle, (uv_stream_t*) &client->handle);

      // called for every read
      read = [&](uv_stream_t *tcp, ssize_t nread, const uv_buf_t *buf) {
        ssize_t parsed;
        Client *client = static_cast<Client *>(tcp->data);

        if (nread >= 0) {
          parsed = (ssize_t) http_parser_execute(&client->parser,
                                                 &events_.settings,
                                                 buf->base,
                                                 nread);

          // close handle
          if (parsed < nread) {
            uv_close((uv_handle_t*) &client->handle, free_client);
          }
        } else {
          if (nread != UV_EOF) {
            // @TODO - debug error
          }

          // close handle
          uv_close((uv_handle_t*) &client->handle, free_client);
        }

        // free request buffer data
        free(buf->base);
      };

      // allocate memory and attempt to read.
      uv_read_start((uv_stream_t*) &client->handle,
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
