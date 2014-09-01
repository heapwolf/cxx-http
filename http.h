#ifndef __HTTP__
#define __HTTP__

#include "debug.h"
#include "uv.h"
#include "http_parser.h"

#include <unistd.h>
#include <string>
#include <sstream>
#include <iostream>
#include <vector>

#define MAX_WRITE_HANDLES 1000

namespace http {

using namespace std;

//
// TODO
// move these uv items into Server.
//
static uv_loop_t* uv_loop;
static uv_tcp_t server;

typedef struct {
  string url;
  string method;
  string status_code;
  string body;
  vector<const string> headers;
} Request;

class Response : public ostream {

  private:
    typedef function<void (string s)> Callback;
    Callback callback;
    bool hasCallback = false;
    stringstream ss;

    class Buffer : public stringbuf {
      public:
        Response *res;
        Buffer(ostream &str) {}
        int sync () {

          string s = str();
          std::ostringstream ss;
          ss << "HTTP/1.1 " << res->status << " OK\r\n"

             //
             // just hard code headers for now.
             //
             
             << "Content-Type: text/plain\r\n"
             << "Connection: keep-alive\r\n"
             
             //
             // set the content length and content.
             //
             
             << "Content-Length: " << s.size() << "\r\n"
             << "\r\n"
             << s;

          s = ss.str();

          if (res->hasCallback) {
            res->callback(s);
          }
          return 0;
        }
    };

    Buffer buffer;

  public:
    vector<const string> headers;
    int status = 200;

    void onEnd(Callback cb) {
      hasCallback = true;
      callback = cb;
    }

    bool setHeader() {
      //
      // TODO
      // Put headers into a map
      //
      return true;
    }

    void setStatus(int code) {
      status = code;
    }

    Response() :
      ostream(&buffer), 
      buffer(ss) {
        buffer.res = this;
      }
    ~Response() {
    }
};

typedef struct : Request {
  uv_tcp_t handle;
  http_parser parser;
  uv_write_t write_req;
} Client;

typedef struct {
  uv_work_t request;
  Client* client;
  bool error;
  string result;
} Baton;

void free_client(uv_handle_t* handle) {
  auto *client = reinterpret_cast<Client*>(handle->data);
  free(client);
}

typedef function<void (Request &req, Response &res)> Listener;

class HTTPEvents {

  public:
    http_parser_settings settings;
    Listener listener;

    ssize_t exec(
      http_parser *parser, const http_parser_settings *settings,
      const char *data, size_t len) {

        return (ssize_t) http_parser_execute(parser, settings, data, len);
    }

    HTTPEvents(Listener fn) {
      listener = fn;

      static function<int(http_parser* parser)> on_message_complete;
      static function<int(http_parser* parser, const char *at, size_t len)> on_url;

      //
      // called once a connection has been made and the message is complete.
      //
      on_message_complete = 
      [&](http_parser* parser) -> int {

        auto *client = reinterpret_cast<Client*>(parser->data);
        Request req;

        req.url = client->url;
        req.method = client->method;

        Response res;

        res.onEnd([&](string str) {

          auto *client = reinterpret_cast<Client*>(parser->data);

          uv_buf_t resbuf;
          resbuf.base = (char *) str.c_str();
          resbuf.len = str.size();

          uv_write(
            &client->write_req,
            (uv_stream_t*)&client->handle,
            &resbuf,
            1,
            [](uv_write_t* req, int status) {
              if (!uv_is_closing((uv_handle_t*)req->handle)) {
                //
                // move this out to user-space
                //
                // Baton *baton = static_cast<Baton *>(req->data);
                // delete baton;
                uv_close((uv_handle_t*) req->handle, free_client);
              }
            }
          );
        });

        listener(req, res);
        return 0;
      };

      //
      // called after the url has been parsed.
      //
      settings.on_url = 
      [](http_parser* parser, const char *at, size_t len) -> int {
        
        auto *client = reinterpret_cast<Client*>(parser->data);
        
        if (at && client) {
          client->url = std::string(at, len);
        }
        return 0; 
      };

      //
      // called when there are either fields or values in the request.
      //
      settings.on_header_field = 
      [](http_parser* parser, const char* at, size_t length) -> int {
        return 0;
      };

      settings.on_header_value = 
      [](http_parser* parser, const char* at, size_t length) -> int {
        return 0;
      };

      //
      // called once all fields and values have been parsed.
      //
      settings.on_headers_complete = 
      [](http_parser* parser) -> int {
      
        auto *client = reinterpret_cast<Client*>(parser->data);
        client->method = string(http_method_str((enum http_method) parser->method));
        return 0; 
      };

      //
      // called when there is a body for the request.
      //
      settings.on_body =
      [](http_parser* parser, const char* at, size_t len) -> int { 
        
        auto *client = reinterpret_cast<Client*>(parser->data);
 
        if (at && client) {
          client->body = std::string(at, len);
        }
        return 0; 
      };

      //
      // called after all other events.
      //
      settings.on_message_complete = 
      [](http_parser* parser) -> int {
        return on_message_complete(parser);
      };
    }
};

class Server {

  public:
    HTTPEvents events;

    //
    // called by the user when they want to start listening for
    // connections. starts the main event loop, provides parser, etc.
    //
    int listen(const char* ip, int port) {

      int cores = sysconf(_SC_NPROCESSORS_ONLN);
      char cores_string[10];
      sprintf(cores_string, "%d", cores);
      setenv("UV_THREADPOOL_SIZE", cores_string, 1);

      uv_loop = uv_default_loop();
      uv_tcp_init(uv_loop, &server);

      //
      // Not sure exactly how to use this,
      // after the initial timeout, it just
      // seems to kill the server.
      //
      //uv_tcp_keepalive(&server,1,60);

      struct sockaddr_in address;
      uv_ip4_addr(ip, port, &address);
      uv_tcp_bind(&server, (const struct sockaddr*) &address, 0);

      static function<void(uv_stream_t* server, int status)> on_connect;
      static function<void(uv_stream_t* tcp, ssize_t nread, const uv_buf_t * buf)> read;

      //
      // called once a connection is made.
      //
      on_connect = [&](uv_stream_t* server_handle, int status) {

        Client *client = new Client();

        uv_tcp_init(uv_loop, &client->handle);
        http_parser_init(&client->parser, HTTP_REQUEST);

        client->parser.data = client;
        client->handle.data = client;

        uv_accept(server_handle, (uv_stream_t*)&client->handle);

        //
        // called for every read.
        //
        read = [&](uv_stream_t* tcp, ssize_t nread, const uv_buf_t * buf) {

          ssize_t parsed;
          auto *client = reinterpret_cast<Client*>(tcp->data);

          if (nread >= 0) {
            parsed = events.exec(&client->parser, &events.settings, buf->base, nread);

            if (parsed < nread) {
              uv_close((uv_handle_t*) &client->handle, free_client);
            }
          } 
          else {

            if (nread != UV_EOF) {
              // debug error
            }
            uv_close((uv_handle_t*) &client->handle, free_client);
          }
          free(buf->base);
        };

        //
        // allocate memory and attempt to read.
        //
        uv_read_start(
          (uv_stream_t*) &client->handle, 
          [](uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf) {
            *buf = uv_buf_init((char*) malloc(suggested_size), suggested_size); 
          }, 
          [](uv_stream_t* tcp, ssize_t nread, const uv_buf_t * buf) {
            read(tcp, nread, buf);
          }
        );
      };

      uv_listen(
        (uv_stream_t*) &server, 
        MAX_WRITE_HANDLES, 
        [](uv_stream_t* server, int status) {
          on_connect(server, status);
        }
      );

      uv_run(uv_loop,UV_RUN_DEFAULT);
      return 0;
    }

    Server(Listener listener) :
      events(listener) {

    }
    ~Server() {
      //
      // TODO
      // maybe free the parser instance.
      //
    }
};

} // namespace http

#endif

