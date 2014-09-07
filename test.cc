
#include "http.h"

using namespace http;

int main () {

  Server server([](auto &req, auto &res) {

    res.setStatus(200);
    res.setHeader("Content-Type", "text/plain");
    res.setHeader("Connection", "keep-alive");
    //res.setHeader("Connection", "close");
    //res.setHeader("Transfer-Encoding", "chunked");


    //res << req.method << " " << req.url << endl;

    //res.write("YES");
    //res.write("YES");
    //res.write("YES");
    //res << "end" << endl;
    res.end("OK");

    //res << "OK" <<  endl;

  });

  server.listen("0.0.0.0", 8000);
}

