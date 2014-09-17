{
  'includes': [ 'common.gypi' ],
  'targets': [
    {
      'target_name': 'nodeuv-http',
      'product_name': 'nodeuv-http',
      'type': 'static_library',
      'sources': [
        'http.h', 'http.cc'
      ],
      'include_dirs': [
        './deps/nodeuv-uri',
      ],
      'dependencies': [
        './deps/libuv/uv.gyp:libuv',
        './deps/http-parser/http_parser.gyp:http_parser'
      ],
    },
    {
      'target_name': 'server',
      'type': 'executable',
      'sources': [
        'http.cc', 'server.cc',
      ],
      'include_dirs': [
        './deps/nodeuv-uri',
      ],     
      'dependencies': [
        './deps/libuv/uv.gyp:libuv',
        './deps/http-parser/http_parser.gyp:http_parser',
        'nodeuv-http'
      ],
    },
    {
      'target_name': 'client',
      'type': 'executable',
      'sources': [
        'http.cc', 'client.cc',
      ],
      'include_dirs': [
        './deps/nodeuv-uri',
      ],
      'dependencies': [
        './deps/libuv/uv.gyp:libuv',
        './deps/http-parser/http_parser.gyp:http_parser',
        'nodeuv-http'
      ],
    },
 ],
}

