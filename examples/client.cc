#include "../http.h"

using namespace http;

int main () {

  Client client("http://localhost:8000", [](auto &res) {
    cout << res.body << endl;
  });

}

