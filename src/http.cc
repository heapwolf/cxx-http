#include "../http.h"

namespace http {

  using namespace std;

  const string CRLF = "\r\n";

  void ClientOrServer::read_allocator(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf) {
    *buf = uv_buf_init((char*) malloc(suggested_size), suggested_size);
  }

  void ClientOrServer::read (uv_stream_t* tcp, ssize_t nread, const uv_buf_t* buf) {

    Context* context = static_cast<Context*>(tcp->data);
    ClientOrServer* client = static_cast<ClientOrServer*>(context->instance);

    if (nread >= -1) {
      ssize_t parsed = (ssize_t) http_parser_execute(
        &context->parser, &parser_settings, buf->base, nread);

      if (parsed < nread) {
        uv_close((uv_handle_t*) &context->handle, free_context);
      }
      if (parsed != nread) {
        // @TODO
        // Error Callback
      }
    }
    else {
      if (nread != UV_EOF) {
        // @TODO - debug error
      }
      uv_close((uv_handle_t*) &context->handle, free_context);
    }
    free(buf->base);
  }


  Server::Server (Listener fn) :
    listener(fn) { 

    }


  Client::Client (string ustr, Listener fn) :
    listener(fn) {
      auto u = uri::ParseHttpUrl(ustr);
      opts.host = u.host;
      if (u.port) {
        opts.port = u.port;
      }
      if (!u.path.empty()) {
        opts.url = u.path;
      }

      connect();
    }


  Client::Client (Client::Options o, Listener fn) :
    listener(fn) {
      opts = o;
      listener = fn;
      connect();
    }


  void free_context (uv_handle_t* handle) {
    auto* context = reinterpret_cast<Context*>(handle->data);
    context->writes.clear();
    free(context);
  }


  //
  // Events
  //
  int parser_on_url (http_parser* parser, const char* at, size_t len) {
    Context* context = static_cast<Context*>(parser->data);
    if (at && context) { context->url = string(at, len); }
    return 0;
  }

  int parser_on_header_field(http_parser *parser, const char *at, size_t length) {
    Context *context = static_cast<Context *>(parser->data);
    context->next_header = string(at, length);
    return 0;
  }

  int parser_on_header_value(http_parser *parser, const char *at, size_t length) {
    Context *context = static_cast<Context *>(parser->data);
    context->headers.insert({context->next_header, string(at, length)});
    return 0;
  }

  int parser_on_headers_complete (http_parser* parser) {
    Context* context = static_cast<Context*>(parser->data);
    context->method = string(http_method_str((enum http_method) parser->method));
    return 0;
  }

  int parser_on_body (http_parser* parser, const char* at, size_t len) {
    Context* context = static_cast<Context*>(parser->data);
    if (at && context && (int) len > -1) {
      context->body << string(at, len);
    }
    return 0;
  }

  int parser_on_message_complete (http_parser* parser) {
    Context* context = static_cast<Context*>(parser->data);
    ClientOrServer* instance = static_cast<ClientOrServer*>(context->instance);
    instance->complete(parser);
    return 0;
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

    stringstream ss;

    if (!writtenOrEnded) {

      ss << "HTTP/1.1 " << statusCode << " " << statusAdjective << CRLF;

      for (auto &header : headers) {
        ss << header.first << ": " << header.second << CRLF;
      }
      ss << CRLF;
      writtenOrEnded = true;
    }

    bool isChunked = headers.count("Transfer-Encoding") 
      && headers["Transfer-Encoding"] == "chunked";

    if (isChunked) {
      ss << std::hex << str.size() 
         << std::dec << CRLF << str << CRLF;
    }
    else {
      ss << str;
    }

    if (isChunked && end) {
      ss << "0" << CRLF << CRLF;
    }

    str = ss.str();

    // response buffer
    uv_buf_t resbuf = {
      .base = (char*) str.c_str(),
      .len = str.size()
    };

    Context* context = static_cast<Context*>(this->parser.data);

    auto id = write_count++;

    uv_write_t write_req;
    context->writes.insert({ id, write_req });

    if (end) {

      ended = true;

      uv_write(&context->writes.at(id), (uv_stream_t*) &context->handle, &resbuf, 1,
        [](uv_write_t* req, int status) {
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

}

