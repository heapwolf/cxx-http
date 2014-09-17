#include "http.h"

using namespace http;

int main () {
 
  Client client("http://google.com", [](auto &res) {
    cout << res.body << endl;
  });

}

