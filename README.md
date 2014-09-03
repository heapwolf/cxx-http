# SYNOPSIS

A simple http server backed by [`libuv`](https://github.com/joyent/libuv)
and [`http-parser`](https://github.com/joyent/http-parser).

# EXAMPLE

```cpp

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
    //res << endl;

  });

  // listen on port 8000
  server.listen("0.0.0.0", 8000);

  return 0;
}
```

# PERFORMANCE

Just for fun. Without any real statistical significance, here are
some quick averages from apache ab, run on an old macbook air. Also
neat is that `libuv-http` uses about `400KB` of memory compared to
Node.js' `10-15MB`.

### libuv-http
```
Requests per second:    16333.46 [#/sec] (mean)
```

### node.js
```
Requests per second:    3366.41 [#/sec] (mean)
```

# DEVELOPMENT

## REQUIREMENTS

- clang 3.5
- gyp
- leveldb
- libuv
- http-parser

## DEBUGGING

### VALGRIND

```bash
valgrind --leak-check=yes --track-origins=yes --dsymutil=yes ./server
```

## BUILD

```bash
make
```

