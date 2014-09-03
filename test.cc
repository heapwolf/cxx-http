
#include <signal.h>
#include "http.h"

int main (void) {

  signal(SIGPIPE, SIG_IGN);

  http::Server server([](http::Request &req, http::Response &res) {

    res.setStatus(200);
    res.setHeader("Content-Type", "text/plain");
    res.setHeader("Connection", "keep-alive");

    res.write("hi !\n");
    res << req.method;
    res << " ";
    res << req.url;

    res.end();
    exit(0);
  });

  server.listen("0.0.0.0", 8000);
}

