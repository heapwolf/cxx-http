git clone https://github.com/joyent/libuv.git deps/libuv
cd deps/libuv
git checkout v0.11.29
cd..

git clone https://github.com/hij1nx/nodeuv-uri.git deps/nodeuv-uri
cd..

git clone https://github.com/joyent/http-parser.git deps/http-parser
cd deps/http-parser  
git checkout v2.3
cd ..

gyp --depth=. --generator-output=build -Duv_library=shared_library
