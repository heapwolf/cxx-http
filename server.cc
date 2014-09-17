#include "http.h"

using namespace http;

int main () {
 
  Server server([](auto &req, auto &res) {

    res.setStatus(200);
    res.setHeader("Connection", "keep-alive");
    res.setHeader("Content-Type", "text/plain");
    res.setHeader("Transfer-Encoding", "chunked");

    //res.write("OK THANK A LOT");

    //res << "OK: " << req.method << " " << req.url << endl;
    res.end("OK");
  });

  server.listen("0.0.0.0", 8000);

}

