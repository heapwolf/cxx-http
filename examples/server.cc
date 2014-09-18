#include "../http.h"

using namespace http;

int main () {

  Server server([](auto &req, auto &res) {

    res.setStatus(200);
    res.setHeader("Connection", "keep-alive");
    res.setHeader("Content-Type", "text/plain");
    res.setHeader("Transfer-Encoding", "chunked");

    res << "OK: " << req.method << " " << req.url << endl;
  });

  server.listen("0.0.0.0", 8000);

}

