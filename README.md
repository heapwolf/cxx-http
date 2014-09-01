# SYNOPSIS

A very fast, simple URL shortener service using leveldb and libuv.

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
valgrind --leak-check=yes --track-origins=yes --dsymutil=yes ./webserver
```

## BUILD

```bash
make
```

# TODO

- Add leveldb
- Move all deps to `clibs` where possible.

