
all: ./build ./server

./deps/gyp:
	git clone --depth 1 https://chromium.googlesource.com/external/gyp.git ./deps/gyp

./deps/http-parser:
	git clone --depth 1 git://github.com/joyent/http-parser.git ./deps/http-parser

./deps/libuv:
	git clone --depth 1 git://github.com/joyent/libuv.git ./deps/libuv

./build: ./deps/gyp ./deps/libuv ./deps/http-parser
	deps/gyp/gyp --depth=. -Goutput_dir=./out -Icommon.gypi --generator-output=./build -Dlibrary=static_library -Duv_library=static_library -f make -debug

distclean:
	make clean
	rm -rf ./build

clean:
	rm -rf ./build/out/Release/obj.target/server/
	rm -f ./build/out/Release/server
	rm -f ./server

.PHONY: test
