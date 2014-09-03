#include <signal.h>
#include "http.h"

using namespace std;
using namespace http;

int main() {

#ifndef _WIN32
  signal(SIGPIPE, SIG_IGN);
#endif

  Server hs([](Request &req, Response &res) {

    res.setStatus(200);
    res.setHeader("Content-Type", "text/plain");
    res.setHeader("Connection", "keep-alive");
    res << req.method << " " << req.url << endl;

  });

  hs.listen("0.0.0.0", 8000);
}

