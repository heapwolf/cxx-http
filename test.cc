#include "http.h"

using namespace std;
using namespace http;

int main() {

  Server hs([](auto &req, auto &res) {

    res.setStatus(200);
    res.setHeader("Content-Type", "text/plain");
    res.setHeader("Connection", "keep-alive");
 
    res << "HELLO" << endl;

    res.write("WRITE1");
    res.write("WRITE2");
    res.end("END"); 
   //res << req.method << " " << req.url << endl;
 
  });
  
  hs.listen("0.0.0.0", 8000);
}

