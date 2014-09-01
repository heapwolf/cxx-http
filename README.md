# SYNOPSIS

A very fast, simple URL shortener service using leveldb and libuv.

Just for fun. Without any real statistical significance, here are 
some quick averages. Also, Redirector uses about `400KB` of memory
compared to Node.js' `10-15MB`.

### Redirector
```
Requests per second:    16333.46 [#/sec] (mean)
```

### Node.js
```
Requests per second:    3366.41 [#/sec] (mean)
```


# MOTIVATION

A drop in replacement for bit.ly

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

# TODO

- Add leveldb
- Move all deps to `clibs` where possible.

