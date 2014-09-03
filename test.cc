
#include <signal.h>
#include "http.h"

int
main (void) {

  signal(SIGPIPE, SIG_IGN);

  http::Server server([](http::Request &req, http::Response &res) {

    res.setStatus(200);
    res.setHeader("Content-Type", "text/plain");
    res.setHeader("Connection", "keep-alive");

    // output
    res.write("hi !\n");
    // or
    res << req.method;
    res << " ";
    res << req.url;

    // end
    res.end();
    // or
    //res << std::endl;

    exit(0);
  });

  // listen on port 8000
  server.listen("0.0.0.0", 8000);

  return 0;
}

