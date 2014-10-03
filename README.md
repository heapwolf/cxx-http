# SYNOPSIS

A simple http server backed by [`libuv`](https://github.com/joyent/libuv)
and [`http-parser`](https://github.com/joyent/http-parser).

# EXAMPLE

## SERVER ([API](https://github.com/hij1nx/nodeuv-http/wiki/Server-Documentation))
```cpp

#include "http.h"

using namespace http;

int main () {

  Server server([](auto &req, auto &res) {

    res.setStatus(200);
    res.setHeader("Content-Type", "text/plain");
    res.setHeader("Connection", "keep-alive");

    res << req.method << " " << req.url << endl;

  });

  server.listen("0.0.0.0", 8000);
}
```

## CLIENT ([API](https://github.com/hij1nx/nodeuv-http/wiki/Client-Documentation))
```cpp

#include "http.h"

using namespace http;

int main () {

  Client client("http://google.com", [](auto &res) {
    cout << res.body << endl;
  });

}
```

# PERFORMANCE

Just for fun. Without any real statistical significance, here are
some quick averages from apache ab, run on an old macbook air. Also
neat is that `nodeuv-http` server uses about `400KB` of memory 
(compared to Node.js' `10-15MB`).

### nodeuv-http
```
Requests per second:    16333.46 [#/sec] (mean)
```

### node.js
```
Requests per second:    3366.41 [#/sec] (mean)
```

# COMPATIBILITY
Runs on `OSX`, `Linux` and `Windows`.

# DEVELOPMENT

## REQUIREMENTS 

### OSX, Linux
This project is made to work with C++11/14. So you'll either need
fairly recent, stable releases of GCC (`~4.8.x`) or Clang (`~4.2.x`).

### Windows
Windows doesn't have a compatible make tool. So you'll have to run 
this `build.bat` file to clone repos and run `gyp`.

## DEBUGGING

### VALGRIND

```bash
valgrind --leak-check=yes --track-origins=yes --dsymutil=yes ./server
```

## BUILD

```bash
make
```

