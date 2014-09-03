
#include "http.h"

using namespace http;

int main (void) {

  Server server([](auto &req, auto &res) {

    res.setStatus(200);
    res.setHeader("Content-Type", "text/plain");
    res.setHeader("Connection", "keep-alive");
    res << req.method << " " << req.url << endl;

  });

  server.listen("0.0.0.0", 8000);
}

