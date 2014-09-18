
DEPS ?= gyp nodeuv-uri http-parser libuv

all: build test

.PHONY: deps
deps: $(DEPS)

gyp: deps/gyp
deps/gyp:
	git clone --depth 1 https://chromium.googlesource.com/external/gyp.git ./deps/gyp

nodeuv-uri: deps/nodeuv-uri
deps/nodeuv-uri:
	git clone --depth 1 http://github.com/hij1nx/nodeuv-uri ./deps/nodeuv-uri

http-parser: deps/http-parser
deps/http-parser:
	git clone --depth 1 git://github.com/joyent/http-parser.git ./deps/http-parser

libuv: deps/libuv
deps/libuv:
	git clone --depth 1 git://github.com/joyent/libuv.git ./deps/libuv

build: $(DEPS)
	deps/gyp/gyp --depth=. -Goutput_dir=./out -Icommon.gypi --generator-output=./build -Dlibrary=static_library -Duv_library=static_library -f make

.PHONY: test
test: client.cc server.cc
	make -C ./build/ client
	make -C ./build/ server
	cp ./build/out/Release/client ./client
	cp ./build/out/Release/server ./server

clean:
	rm -rf build/

